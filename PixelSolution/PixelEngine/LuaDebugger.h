#pragma once

// The UI only receives a value snapshot. All Lua API calls stay on the engine thread.
#include <algorithm>
#include <atomic>
#include <chrono>
#include <charconv>
#include <filesystem>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "json.hpp"
extern "C" {
#include "Import/lua.h"
#include "Import/luajit.h"
}

namespace LuaDebugger
{
    using Callback = int (__cdecl*)(const char*); // 0: continue, 1: step into, 2: detach
    inline std::mutex configMutex;
    inline Callback callback = nullptr;
    inline std::unordered_map<std::string, std::set<int>> breakpoints;
    inline std::atomic<bool> enabled{ false };
    inline thread_local bool inFrame = false;
    inline thread_local bool step = false;
    inline thread_local lua_State* stepState = nullptr;
    inline thread_local double pausedSeconds = 0;
    inline thread_local void (*onResume)(double) = nullptr;
    // Accessed only on the owning engine thread, between Lua calls.
    inline lua_State* mainState = nullptr;
    inline bool installed = false;
    inline bool restoreJit = false;
    inline lua_Hook previousHook = nullptr;
    inline int previousMask = 0;
    inline int previousCount = 0;

    inline std::string Normalize(const char* source)
    {
        if (!source || !*source) return {};
        if (*source == '@') ++source;
        try
        {
            std::string utf8(source);
            auto path = std::filesystem::absolute(std::filesystem::path(std::u8string(utf8.begin(), utf8.end()))).lexically_normal();
            auto value = path.generic_u8string();
            std::string result(value.begin(), value.end());
            // Windows ASCII path case and separator normalization; retain UTF-8 bytes.
            for (auto& ch : result) if (ch >= 'A' && ch <= 'Z') ch += 'a' - 'A';
            return result;
        }
        catch (...) { return {}; }
    }

    inline void SetBreakpoint(const char* path, int line, bool value)
    {
        auto key = Normalize(path);
        if (key.empty() || line < 1) return;
        std::lock_guard<std::mutex> guard(configMutex);
        if (value) breakpoints[key].insert(line);
        else if (auto it = breakpoints.find(key); it != breakpoints.end())
        {
            it->second.erase(line);
            if (it->second.empty()) breakpoints.erase(it);
        }
    }

    inline void Register(Callback value)
    {
        std::lock_guard<std::mutex> guard(configMutex);
        callback = value;
        if (!value) enabled = false;
    }

    inline std::string Describe(lua_State* state, int index)
    {
        switch (lua_type(state, index))
        {
        case LUA_TNIL: return "nil";
        case LUA_TBOOLEAN: return lua_toboolean(state, index) ? "true" : "false";
        case LUA_TNUMBER:
        {
            char buffer[64];
            auto converted = std::to_chars(buffer, buffer + sizeof(buffer), lua_tonumber(state, index));
            return converted.ec == std::errc{} ? std::string(buffer, converted.ptr) : "number";
        }
        case LUA_TSTRING:
        {
            size_t size = 0;
            const char* text = lua_tolstring(state, index, &size);
            std::string result(text, (std::min)(size, size_t(512)));
            if (size > 512) result += "...";
            return nlohmann::json(result).dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
        }
        default:
            // Never call tostring/__tostring or index metamethods while inspecting.
            return lua_typename(state, lua_type(state, index));
        }
    }

    inline nlohmann::json Variable(lua_State* state, int index, const std::string& name,
        int depth, int& budget, std::unordered_set<const void*>& ancestors)
    {
        --budget;
        const int absolute = index > 0 ? index : lua_gettop(state) + index + 1;
        nlohmann::json node = { {"name", name}, {"type", lua_typename(state, lua_type(state, absolute))},
            {"value", Describe(state, absolute)}, {"children", nlohmann::json::array()} };
        if (!lua_istable(state, absolute)) return node;
        const void* identity = lua_topointer(state, absolute);
        if (ancestors.count(identity)) { node["value"] = "table (cycle)"; return node; }
        if (depth >= 3 || budget <= 0) { node["value"] = "table (preview limit)"; return node; }
        ancestors.insert(identity);
        lua_pushnil(state);
        int count = 0;
        while (lua_next(state, absolute))
        {
            node["children"].push_back(Variable(state, -1, "[" + Describe(state, -2) + "]", depth + 1, budget, ancestors));
            lua_pop(state, 1);
            if (++count >= 64 || budget <= 0)
            {
                lua_pop(state, 1); // remaining key
                node["value"] = "table (preview may be truncated)";
                break;
            }
        }
        ancestors.erase(identity);
        return node;
    }

    inline std::string Snapshot(lua_State* state)
    {
        nlohmann::json result = { {"frames", nlohmann::json::array()} };
        int budget = 2048;
        std::unordered_set<const void*> ancestors;
        for (int level = 0; level < 32 && budget > 0; ++level)
        {
            lua_Debug frame{};
            if (!lua_getstack(state, level, &frame)) break;
            lua_getinfo(state, "nSl", &frame);
            const bool file = frame.source && frame.source[0] == '@';
            nlohmann::json entry = { {"name", frame.name ? frame.name : "(chunk)"},
                {"source", file ? Normalize(frame.source) : (frame.short_src)}, {"file", file},
                {"line", frame.currentline}, {"variables", nlohmann::json::array()} };
            for (int i = 1; i <= 128 && budget > 0; ++i)
            {
                const char* name = lua_getlocal(state, &frame, i);
                if (!name) break;
                if (name[0] != '(')
                    entry["variables"].push_back(Variable(state, -1, std::string("local: ") + name, 0, budget, ancestors));
                lua_pop(state, 1);
            }
            if (lua_getinfo(state, "f", &frame))
            {
                for (int i = 1; i <= 128 && budget > 0; ++i)
                {
                    const char* name = lua_getupvalue(state, -1, i);
                    if (!name) break;
                    entry["variables"].push_back(Variable(state, -1, std::string("upvalue: ") + name, 0, budget, ancestors));
                    lua_pop(state, 1);
                }
                lua_pop(state, 1);
            }
            result["frames"].push_back(std::move(entry));
        }
        result["truncated"] = budget <= 0;
        return result.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    }

    inline void Hook(lua_State* state, lua_Debug* location)
    {
        if (!inFrame || !enabled || location->event != LUA_HOOKLINE) return;
        if (!lua_getinfo(state, "Sl", location) || location->currentline < 1 ||
            !location->source || location->source[0] != '@') return;
        Callback notify = nullptr;
        {
            std::lock_guard<std::mutex> guard(configMutex);
            auto it = breakpoints.find(Normalize(location->source));
            bool hit = it != breakpoints.end() && it->second.count(location->currentline);
            if (!hit && !(step && stepState == state)) return;
            notify = callback;
        }
        if (!notify) return;
        step = false;
        const int top = lua_gettop(state);
        const auto start = std::chrono::steady_clock::now();
        try
        {
            auto snapshot = Snapshot(state);
            // Callback displays a modal inspector; Lua remains on this exact stack frame.
            const int command = notify(snapshot.c_str());
            step = command == 1;
            stepState = step ? state : nullptr;
            if (command == 2) enabled = false;
        }
        catch (...) { enabled = false; }
        lua_settop(state, top);
        const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        pausedSeconds += elapsed;
        if (onResume) onResume(elapsed);
    }

    inline void Attach(lua_State* state)
    {
        mainState = state;
        installed = false;
        step = false;
        stepState = nullptr;
    }

    inline void Prepare()
    {
        if (!mainState) return;
        if (enabled && !installed)
        {
            const int top = lua_gettop(mainState);
            lua_getglobal(mainState, "jit");
            restoreJit = false;
            if (lua_istable(mainState, -1))
            {
                lua_getfield(mainState, -1, "status");
                if (lua_isfunction(mainState, -1) && lua_pcall(mainState, 0, 1, 0) == 0)
                    restoreJit = lua_toboolean(mainState, -1) != 0;
            }
            lua_settop(mainState, top);
            luaJIT_setmode(mainState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
            luaJIT_setmode(mainState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_FLUSH);
            previousHook = lua_gethook(mainState);
            previousMask = lua_gethookmask(mainState);
            previousCount = lua_gethookcount(mainState);
            lua_sethook(mainState, Hook, LUA_MASKLINE, 0);
            installed = true;
        }
        else if (!enabled && installed)
        {
            lua_sethook(mainState, previousHook, previousMask, previousCount);
            if (restoreJit) luaJIT_setmode(mainState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
            installed = false;
            step = false;
            stepState = nullptr;
        }
    }

    struct FrameScope
    {
        explicit FrameScope(bool play, void (*resume)(double) = nullptr)
        { Prepare(); inFrame = play; pausedSeconds = 0; onResume = resume; }
        ~FrameScope() { inFrame = false; onResume = nullptr; }
    };
}
