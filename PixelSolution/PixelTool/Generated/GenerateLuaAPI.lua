---@meta 

---@class Engine
Engine ={}
---@param arg0 string
---@return GameObject
function Engine.CreateGameObject(arg0) end


---@class Scene
Scene ={}
---@param arg0 string
---@return void
function Scene.ChangeScene(arg0) end


---@class Asset
Asset ={}
---@param arg0 string
---@return void
function Asset.Import(arg0) end


---@class Input
Input ={}
---@param arg0 number
---@return boolean
function Input.GetKey(arg0) end

---@param arg0 number
---@return boolean
function Input.GetKeyDown(arg0) end

---@param arg0 number
---@return boolean
function Input.GetKeyUp(arg0) end

---@return number
function Input.GetMousePosition_X() end

---@return number
function Input.GetMousePosition_Y() end


---@class PVector3
---@field X number
---@field Y number
---@field Z number
PVector3 ={}

---@class PixelObject
PixelObject ={}

---@class GameObject
GameObject = {} 
---@generic T 
---@param arg1 `T` 
---@return T 
function GameObject:AddModule(arg1) end 
---@generic T 
---@param arg1 `T` 
---@return T 
function GameObject:GetModule(arg1) end 

---@class Module
Module ={}

---@class Transform
---@field Position PVector3
---@field Rotation PVector3
---@field Scale PVector3
Transform ={}

---@class Camera
Camera ={}

---@class LuaScript
LuaScript ={}
---@param arg0 string
---@return void 
function LuaScript:Register(arg0) end


---@class DebugCamera
DebugCamera ={}

---@class Renderer2D
Renderer2D ={}
---@param arg0 string
---@return void 
function Renderer2D:SetTexture(arg0) end

