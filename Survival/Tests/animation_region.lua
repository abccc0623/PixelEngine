-- Run from Survival with Lua 5.1/LuaJIT; native DLL is deliberately mocked.
local previousFFI = package.loaded.ffi
local previousAnimation = Animation2D
local methods, received
local dll = {}
function dll.Animation2D_CreateRegion(...)
    received = {...}
    return 3
end
function dll.Animation2D_Create(...)
    received = {...}
    return 1
end
local ffiMock = {}
function ffiMock.load(name)
    assert(name == "PixelEngine")
    return dll
end
function ffiMock.cdef(definition)
    assert(definition:find("Animation2D_CreateRegion", 1, true))
end
function ffiMock.metatype(name, mt)
    assert(name == "Animation2DData")
    methods = mt.__index
end
package.loaded.ffi = ffiMock
dofile("Engine/Animation2D.lua")
local animation = setmetatable({thisID = 42}, {__index = methods})
assert(animation:CreateRegion("magic", 192, 48, 24, 24, 8, 1) == 3)
local expected = {42, "magic", 192, 48, 24, 24, 8, 1}
for i, value in ipairs(expected) do
    assert(received[i] == value, "CreateRegion argument " .. i)
end
assert(animation:Create("book", 9, 1, 1) == 1)
assert(received[1] == 42 and received[2] == "book" and received[3] == 9 and received[4] == 1 and received[5] == 1)
package.loaded.ffi = previousFFI
Animation2D = previousAnimation
print("animation_region: Lua wrapper passed (not a native rendering test)")
