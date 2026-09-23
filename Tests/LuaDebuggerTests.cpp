#include <cassert>
#include <iostream>
#include <thread>
#include "LuaDebugger.h"
extern "C" {
#include "lauxlib.h"
#include "lualib.h"
}

using nlohmann::json;
static int hits = 0;
static int command = 0;
static json snapshot;
static int __cdecl OnBreak(const char* text)
{
    ++hits;
    snapshot = json::parse(text);
    return command;
}
static void Run(lua_State* state, const char* script, const char* source)
{
    const int top = lua_gettop(state);
    assert(luaL_loadbuffer(state, script, strlen(script), source) == 0);
    int result = lua_pcall(state, 0, 0, 0);
    if (result != 0) std::cerr << lua_tostring(state, -1) << std::endl;
    assert(result == 0);
    assert(lua_gettop(state) == top);
}
static json FindVariable(const char* name)
{
    for (const auto& variable : snapshot["frames"][0]["variables"])
        if (variable["name"] == name) return variable;
    assert(false && "missing local");
    return {};
}
int main()
{
    auto state = luaL_newstate();
    luaL_openlibs(state);
    LuaDebugger::Attach(state);
    LuaDebugger::Register(OnBreak);
    // Configuration can be updated on another thread without touching the Lua state.
    std::thread configure([] { LuaDebugger::SetBreakpoint("tests/fixture.lua", 5, true); });
    configure.join();
    LuaDebugger::enabled = true;
    const char* script = "local health = 42\nlocal obj = {score=9}\nobj.self=obj\nlocal flag=false\nhealth=health+1\nhealth=health+1\n";
    // No pause in editor/startup calls outside a render frame.
    Run(state, script, "@tests/fixture.lua");
    assert(hits == 0);
    {
        LuaDebugger::FrameScope frame(true);
        Run(state, script, "@tests/FIXTURE.lua");
        assert(hits == 1);
        assert(snapshot["frames"][0]["line"] == 5);
        assert(FindVariable("local: health")["value"] == "42");
        assert(FindVariable("local: flag")["value"] == "false");
        assert(FindVariable("local: obj").dump().find("cycle") != std::string::npos);
    }
    command = 1;
    {
        LuaDebugger::FrameScope frame(true);
        Run(state, script, "@tests/fixture.lua");
        assert(hits == 3); // breakpoint on line 5, then step to line 6
        assert(snapshot["frames"][0]["line"] == 6);
        assert(FindVariable("local: health")["value"] == "43");
    }
    LuaDebugger::step = false;
    command = 0;
    LuaDebugger::SetBreakpoint("tests/fixture.lua", 5, false);
    LuaDebugger::SetBreakpoint("tests/coroutine.lua", 4, true);
    {
        LuaDebugger::FrameScope frame(true);
        Run(state, "local co=coroutine.create(function()\nlocal n=7\ncoroutine.yield()\nn=n+1\nend)\ncoroutine.resume(co)\ncoroutine.resume(co)\n", "@tests/coroutine.lua");
        assert(hits == 4);
        assert(FindVariable("local: n")["value"] == "7");
    }
    LuaDebugger::SetBreakpoint("tests/coroutine.lua", 4, false);
    LuaDebugger::SetBreakpoint("tests/upvalue.lua", 5, true);
    {
        LuaDebugger::FrameScope frame(true);
        Run(state, "local outer=12\nlocal function fn()\nlocal value=outer\nvalue=value+1\nvalue=value+1\nend\nfn()\n", "@tests/upvalue.lua");
        assert(hits == 5);
        assert(FindVariable("upvalue: outer")["value"] == "12");
        assert(snapshot["frames"].size() == 2);
    }
    // Inspection must not execute __tostring or __index, even on cyclic/large tables.
    LuaDebugger::SetBreakpoint("tests/safe.lua", 5, true);
    {
        LuaDebugger::FrameScope frame(true);
        Run(state, "local object=setmetatable({}, {__tostring=function() error('unsafe') end, __index=function() error('unsafe') end})\nlocal bytes=string.char(255,0)\nlocal many={}\nfor i=1,10000 do many[i]=i end\nlocal done=true\n", "@tests/safe.lua");
        assert(hits == 6);
        assert(FindVariable("local: many")["children"].size() == 64);
    }
    command = 2;
    LuaDebugger::SetBreakpoint("tests/fixture.lua", 5, true);
    {
        LuaDebugger::FrameScope frame(true);
        Run(state, script, "@tests/fixture.lua");
        assert(hits == 7 && !LuaDebugger::enabled);
        Run(state, script, "@tests/fixture.lua");
        assert(hits == 7);
    }
    LuaDebugger::Prepare();
    assert(!LuaDebugger::installed);
    assert(lua_gethook(state) == nullptr);
    LuaDebugger::Register(nullptr);
    LuaDebugger::Attach(nullptr);
    lua_close(state);
    std::cout << "PASS: breakpoints, locals, tables/cycles, step, coroutine, upvalues, stack preservation, safe preview, detach\n";
}
