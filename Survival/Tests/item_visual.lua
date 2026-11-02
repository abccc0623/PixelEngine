local images, transforms, animations, active = {}, {}, {}, {}
local nextID, created = 10, 0
function PVector3(x,y,z)
    return {x = x,y = y,z = z}
end
Entity = {}
function Entity.CreateEmpty()
    nextID = nextID + 1
    return nextID
end
function Entity.SetActive(id,value)
    active[id] = value
end
Transform = {}
function Transform.Add(id)
    local transform = {}
    function transform:SetParent(parentID)
        self.Parent = parentID
    end
    transforms[id] = transform
    return transform
end
function Transform.Get(id)
    return transforms[id]
end
UIImage = {}
function UIImage.Add(id)
    local image = {thisID = id}
    function image:SetTexture(value)
        self.Texture = value
    end
    function image:SetColor(r,g,b,a)
        self.Alpha = a
    end
    function image:SetPivot()


    end
    function image:SetOrder()


    end
    function image:SetAspectFit(w,h)
        self.FitWidth, self.FitHeight = w,h
    end
    function image:SetTextureSize(w,h)
        self.FitWidth, self.FitHeight = w,h
    end
    function image:SetContentAspectFit(w,h)
        self.FitWidth, self.FitHeight = w,h
    end
    images[id] = image
    return image
end
function UIImage.Get(id)
    return images[id]
end
Animation2D = {}
function Animation2D.Add(id)
    local animation = {}
    function animation:Create(texture,x,y,speed)
        created = created + 1
        self.Texture, self.X, self.Y, self.Speed = texture,x,y,speed
        return created
    end
    function animation:CreateRegion(texture,startX,startY,w,h,count,speed)
        created = created + 1
        self.Region = {texture,startX,startY,w,h,count,speed}
        return created
    end
    function animation:SetLoop(index,value)
        self.Loop = value
    end
    function animation:SetUseUnscaledTime(value)
        self.Unscaled = value
    end
    function animation:Play(index)
        self.Index = index
    end
    function animation:Reset()
        self.Resets = (self.Resets or 0) + 1
    end
    animations[id] = animation
    return animation
end
function Animation2D.Get(id)
    return animations[id]
end
dofile("D:/PixelEngine/Survival/Asset/Table/ItemVisual.lua")
local image = UIImage.Add(1)
ItemVisual.Apply(image,{Texture = "static"},26,26,true)
assert(image.Texture == "static" and image.Alpha == 255 and created == 0)
local item = {Texture = "magic_sheet",FramesX = 8,FramesY = 4,AnimationSpeed = 2}
ItemVisual.Apply(image,item,26,26,true)
local child = ItemVisual.Icons[1].ID
assert(image.Alpha == 0 and active[child] and animations[child].X == 8 and animations[child].Y == 4)
assert(animations[child].Speed == 2 and animations[child].Loop and animations[child].Unscaled)
assert(images[child].FitWidth == 208 and images[child].FitHeight == 104)
assert(transforms[child].scale.x == 1 / 8 and transforms[child].scale.y == 1 / 4)
ItemVisual.Apply(image,item,26,26,false)
assert(created == 1 and animations[child].Resets == 1 and not animations[child].Unscaled)
ItemVisual.Apply(image,{Texture = "another_static"},18,18,false)
assert(image.Alpha == 255 and image.Texture == "another_static" and images[child].Alpha == 0 and not active[child])
-- Parent activation must not accidentally reveal an old animated child.
Entity.SetActive(child,true)
assert(images[child].Alpha == 0)
ItemVisual.Apply(image,item,32,32,true)
assert(created == 1 and images[child].Alpha == 255)
ItemVisual.Apply(image,nil,24,24,true)
assert(image.Alpha == 0 and images[child].Alpha == 0)
ItemVisual.Apply(image,{Texture = "incomplete",FramesX = 8},24,24,true)
assert(image.Texture == "incomplete" and created == 1)
ItemVisual.Apply(image,item,18,18,false,"road_2")
assert(image.Texture == "road_2" and image.Alpha == 255 and images[child].Alpha == 0)
dofile("D:/PixelEngine/Survival/Asset/Table/ItemList.lua")
ItemVisual.Apply(image,ItemList[2],18,18,false)
local region = animations[child].Region
assert(region[1] == "item_00" and region[2] == 192 and region[3] == 0)
assert(region[4] == 24 and region[5] == 24 and region[6] == 8 and region[7] == 1)
assert(images[child].FitWidth == 18 and images[child].FitHeight == 18)
assert(transforms[child].scale.x == 1 and transforms[child].scale.y == 1)
assert(image.Alpha == 0 and active[child] and created == 2)
ItemVisual.Apply(image,ItemList[2],32,32,true)
assert(created == 2 and images[child].FitWidth == 32 and animations[child].Unscaled)
dofile("D:/PixelEngine/Survival/Asset/Table/Graph.lua")
Graph.SetRoadImage(image,ItemList[2].Texture,ItemList[2])
assert(animations[child].Unscaled, "Grid icon must animate while game time is paused")
Graph.SetRoadImage(image,ItemList[3].Texture,ItemList[3])
region = animations[child].Region
assert(region[1] == "item_01" and region[2] == 192 and region[3] == 0)
assert(region[4] == 24 and region[5] == 24 and region[6] == 8)
assert(animations[child].Unscaled and images[child].FitWidth == 18)
print("PASS: static/sheet/region icons, start point atlas selection, sizing, caching and pause mode")
for _, index in ipairs({7,8}) do
    local reactor = ItemList[index]
    assert(reactor.FrameCount == nil and reactor.FramesX == nil)
    ItemVisual.Apply(image,reactor,24,24,true)
    assert(image.Texture == reactor.Texture and image.Alpha == 255)
    assert(not active[child] and images[child].Alpha == 0)
end
print("PASS: static purchased reactor runes replace animated icons")
assert(FactoryType.Generator ~= FactoryType.Distribution and FactoryType.Generator ~= FactoryType.Converter)
for _, index in ipairs({1,5,6,9}) do
    assert(ItemList[index].ItemType == ItemType.Factory and ItemList[index].FactoryType == FactoryType.Distribution)
end
for _, index in ipairs({7,8}) do
    assert(ItemList[index].ItemType == ItemType.Factory and ItemList[index].FactoryType == FactoryType.Converter)
end
for _, item in ipairs(ItemList) do
    assert(item.FactoryType ~= FactoryType.Generator,"Generator implementation is deferred")
end
assert(ItemList[2].ItemType == ItemType.System and ItemList[2].FactoryType == nil)
print("PASS: installation subtypes; generator type reserved without adding gameplay")
