local methods, parameters
local dll = {}
function dll.Renderer2D_SetTransparent(...)
    parameters = {...}
end
local mock = {}
function mock.load()
    return dll
end
function mock.cdef(definition)
    assert(definition:find("Renderer2D_SetTransparent",1,true))
end
function mock.metatype(name,mt)
    methods = mt.__index
end
package.loaded.ffi = mock
dofile("Engine/Renderer2D.lua")
local renderer = setmetatable({thisID = 32},{__index = methods})
renderer:SetTransparent(true)
assert(parameters[1] == 32 and parameters[2] == true)
renderer:SetTransparent(false)
assert(parameters[1] == 32 and parameters[2] == false)
print("PASS: transparent renderer Lua binding arguments (native DLL mocked)")
