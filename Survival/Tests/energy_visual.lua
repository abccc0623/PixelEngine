Grid = {BoardID = 10, Visible = true}
GameFlow = {State = "Combat"}
dofile("Asset/Table/EnergyFunction.lua")
Transform = {}
function Transform.Add(id)
    local transform = {}
    function transform:SetParent(parent)
        self.Parent = parent
    end
    return transform
end
UIImage = {}
function UIImage.Add(id)
    local image = {}
    function image:SetTexture(texture)
        self.Texture = texture
    end
    function image:SetPivot(x,y)
        self.Pivot = {x,y}
    end
    function image:SetTextureSize(w,h)
        self.Size = {w,h}
    end
    function image:SetOrder(order)
        self.Order = order
    end
    function image:SetColor(r,g,b,a)
        self.Alpha = a
    end
    return image
end
Animation2D = {}
function Animation2D.Add(id)
    local animation = {}
    function animation:CreateRegion(...)
        self.Region = {...}
        return 0
    end
    function animation:SetLoop(index,loop)
        self.Loop = loop
    end
    function animation:SetUseUnscaledTime(value)
        self.Unscaled = value
    end
    function animation:Play(index)
        self.Playing = index
    end
    function animation:Reset()
        self.Resets = (self.Resets or 0) + 1
    end
    return animation
end
dofile("Asset/Table/EnergyList.lua")
local script = dofile("Asset/Script/Grid/EnergyItem.pxm")
script.ID = 1
script:Spawn()
script:Setting(1)
local expected = {"item_02",192,24,24,24,8,1}
for i, value in ipairs(expected) do
    assert(script.animation.Region[i] == value)
end
assert(script.animation.Loop and script.animation.Unscaled and script.animation.Playing == 0)
assert(script.image.Size[1] == 15 and script.image.Size[2] == 15)
assert(script.transform.Parent == Grid.BoardID and script.image.Order == 0)
Grid.Visible = false
script:Update()
assert(script.image.Alpha == 0)
Grid.Visible = true
script:Update()
assert(script.image.Alpha == 255)
function PVector2(x,y)
    return {x = x,y = y}
end
function PVector3(x,y,z)
    return {x = x,y = y,z = z}
end
MotionType = {Kinematic = 1}
Rigidbody2D = {}
function Rigidbody2D.Add(id)
    local body = {}
    function body:CreateCircleCollider(radius,offset)
        self.Radius = radius
    end
    function body:SetMotionType(value)
        self.MotionType = value
    end
    function body:SetLayer(value)
        self.Layer = value
    end
    return body
end
Renderer2D = {}
function Renderer2D.Add(id)
    local renderer = {}
    function renderer:SetColor(r,g,b,a)
        self.Color = {r,g,b,a}
    end
    function renderer:SetTransparent(value)
        self.Transparent = value
    end
    return renderer
end
local bullet = dofile("Asset/Script/Entity/Bullet.pxm")
bullet.ID = 2
bullet:Spawn()
bullet:ActiveSetting(PVector3(1,0,0),1)
for i, value in ipairs(expected) do
    assert(bullet.animation.Region[i] == value, "Bullet must use the arriving energy image")
end
assert(bullet.animation.Loop and bullet.animation.Playing == 0)
assert(bullet.rigidbody.Radius == 0.25 and bullet.rigidbody.Layer == "Bullet" and bullet.speed == 10)
assert(bullet.renderer.Transparent)
EnergyList[2] = {Texture = "item_01",StartX = 0,StartY = 48,FrameWidth = 24,FrameHeight = 24,FrameCount = 8}
script:Setting(2)
bullet:ActiveSetting(PVector3(1,0,0),2)
assert(script.ItemIndex == 2 and bullet.ItemIndex == 2)
assert(script.animation.Region[1] == "item_01" and bullet.animation.Region[1] == "item_01")
assert(script.animation.Region[3] == 48 and bullet.animation.Region[3] == 48)
assert(script.animation.Unscaled and not bullet.animation.Unscaled)
print("PASS: energy atlas region, frame size, looping, real time and Grid visibility (Lua mocks)")
