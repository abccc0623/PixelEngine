local transforms,images,free,active = {},{},{},{}
local created,expansion = 0,0
local function noop()


end
Entity = {SetActive = noop}
function PVector3(x,y,z)
    return {x=x,y=y,z=z}
end
Transform = {Add = function(id)
    transforms[id] = {rotation = {},SetParent = noop}
    return transforms[id]
end,Get = function(id)
    return transforms[id]
end}
UIImage = {Add = function(id)
    images[id] = setmetatable({thisID = id},{__index = function()
        return noop
    end})
    return images[id]
end,Get = function(id)
    return images[id]
end}
Pool = {SetExpansionSize = function(key,count)
    assert(key == "GridCellImage")
    expansion = count
end,Active = function(key)
    if #free == 0 then
        for i=1,expansion do
            created = created + 1
            local child = dofile("Asset/Script/Grid/GridCellImage.pxm")
            child.ID = created
            child:Spawn()
            free[#free+1] = created
        end
    end
    local id = table.remove(free)
    assert(not active[id])
    active[id] = true
    return id
end,Disable = function(key,id)
    assert(active[id])
    active[id] = nil
    free[#free+1] = id
end}
ItemList = {{Texture="road_1"}}
local visualCalls = {}
local playing = {}
Animation2D = {}
function Animation2D.Get(id)
	return {
		Play = function()
			playing[id] = true
		end,
		Stop = function()
			playing[id] = false
		end,
		Reset = noop,
	}
end
ItemVisual = {Icons = {}}
function ItemVisual.Apply(image,item,width,height,unscaled)
	ItemVisual.Icons[image.thisID] = {ID = image.thisID,Animations = {road = 1},Current = "road"}
	visualCalls[#visualCalls + 1] = item.Texture
	assert(item.FrameWidth == 64 and item.FrameHeight == 64 and item.FrameCount == 8)
	assert(width == 72 and height == 72 and unscaled == true)
end
Direction = {Up=1,Down=2,Left=3,Right=4}
Grid = {width=12,height=12,[1] = {[1] = {ItemIndex=1},[2] = {ItemIndex=1}}}
dofile("Asset/Script/Grid/GridGraph.lua")
local s = dofile("Asset/Script/Grid/GridEntity.pxm")
s.ID = 1000
s:Spawn()
assert(created == 50 and #s.DrawnImages == 2)
assert(playing[s.DrawnImages[1]] == false and playing[s.DrawnImages[2]] == false)
Grid[1][2].ItemRotation = 90
s:Draw()
assert(created == 50 and #s.DrawnImages == 2)
assert(transforms[s.DrawnImages[2]].rotation.z == 90)
assert(transforms[s.DrawnImages[2]].position == s.CellPos[1][2])
Grid[1][1].ItemIndex = 0
s:Draw()
assert(created == 50 and #s.DrawnImages == 1 and #free == 49)
Grid[1][2].RoadTexture = "road_2"
s:Draw()
assert(visualCalls[#visualCalls] == "road_2")
Grid[1][2].RoadTexture = "road_3"
s:Draw()
assert(visualCalls[#visualCalls] == "road_3")
local build = GridGraph.Build
function GridGraph.Build()
	build()
	GridGraph.Nodes["1:2"].OnPath = true
end
s:Draw()
assert(playing[s.DrawnImages[1]] == true)
GridGraph.Build = build
s:Draw()
assert(playing[s.DrawnImages[1]] == false)
ItemList[5] = {Texture = "router",FrameWidth = 64,FrameHeight = 64,FrameCount = 8}
Grid[1][2].ItemIndex = 5
s:Draw()
assert(playing[s.DrawnImages[1]] == false)
function GridGraph.Build()
	build()
	GridGraph.Nodes["1:2"].OnPath = true
end
s:Draw()
assert(playing[s.DrawnImages[1]] == true)
GridGraph.Build = build
s:Draw()
assert(playing[s.DrawnImages[1]] == false)
print("grid_image_pool: PASS")
