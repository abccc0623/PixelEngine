local root = "D:/PixelEngine/Survival/Asset/"
dofile(root .. "Table/ItemList.lua")
EnergyFunction = {}
function EnergyFunction.Spawn()
    return false
end
local function Fixture(joinIndex,splitIndex)
    Grid = {width = 5,height = 5,StartPoint = {x = 1,y = 3},EndPoint = {x = 5,y = 3}}
    for x = 1, 5 do
        Grid[x] = {}
        for y = 1, 5 do Grid[x][y] = {ItemIndex = 0} end
    end
    Grid[1][3].ItemIndex = 2
    Grid[2][3].ItemIndex = splitIndex or 5
    Grid[5][3].ItemIndex = 3
    for _, point in ipairs({{2,2},{3,2},{4,2},{2,4},{3,4},{4,4},{4,3}}) do
        Grid[point[1]][point[2]].ItemIndex = 1
    end
    Grid[4][3].ItemIndex = joinIndex
    dofile(root .. "Table/Graph.lua")
    assert(Graph.Reset())
end
local function Count(set)
    local count = 0
    for _ in pairs(set) do count = count + 1 end
    return count
end
local function CheckLimits()
    for key, port in pairs(Graph.Ports) do
        local x,y = key:match("(%d+):(%d+)")
        local inputs, outputs = Graph.PortLimits({x = tonumber(x),y = tonumber(y)})
        assert(Count(port.Inputs) <= inputs and Count(port.Outputs) <= outputs)
        for neighbor in pairs(port.Inputs) do assert(not port.Outputs[neighbor]) end
    end
end

-- Only a merger may receive a second incoming road, regardless of physical adjacency.
for _, index in ipairs({1,5,6,7,8}) do
    Fixture(index)
    assert(#Graph.routes == 1,"Non-merger accepted a second route")
    assert(Count(Graph.Ports["4:3"].Inputs) == 1)
    local upper, lower = Graph.Ports["3:2"],Graph.Ports["3:4"]
    assert((upper ~= nil) ~= (lower ~= nil))
    assert(Grid[3][upper and 4 or 2].RoadTexture == "road_3")
    CheckLimits()
end

Fixture(9)
assert(#Graph.routes == 2)
assert(Count(Graph.Ports["2:3"].Inputs) == 1 and Count(Graph.Ports["2:3"].Outputs) == 2)
assert(Count(Graph.Ports["4:3"].Inputs) == 2 and Count(Graph.Ports["4:3"].Outputs) == 1)
assert(Count(Graph.Ports["1:3"].Inputs) == 0 and Count(Graph.Ports["5:3"].Outputs) == 0)
assert(Grid[3][2].RoadTexture ~= "road_3" and Grid[3][4].RoadTexture ~= "road_3")
CheckLimits()

-- A rejected third inlet or second outlet must not leave partially reserved ports.
Grid[3][3].ItemIndex = 1
assert(not Graph.CanConnect({x = 3,y = 3},{x = 4,y = 3},Graph.Ports))
assert(not Graph.CanConnect({x = 4,y = 3},{x = 3,y = 3},Graph.Ports))
assert(not Graph.CanConnect({x = 2,y = 3},{x = 3,y = 3},Graph.Ports))
assert(not Graph.CanConnect({x = 5,y = 2},{x = 5,y = 3},Graph.Ports))
local before = Graph.Ports
assert(not Graph.ReserveRoute({{x = 3,y = 3},{x = 4,y = 3},{x = 5,y = 3}}))
assert(Graph.Ports == before and Graph.Ports["3:3"] == nil)

-- A normal cell still has one outlet, even when a merger is available downstream.
Fixture(9,1)
assert(#Graph.routes == 1 and Count(Graph.Ports["2:3"].Outputs) == 1)
-- Rebuilding clears old automatic connections.
Grid[2][3].ItemIndex = 5
assert(Graph.Reset() and #Graph.routes == 2)
Grid[4][3].ItemIndex = 1
assert(Graph.Reset() and #Graph.routes == 1)
Grid[4][3].ItemIndex = 9
assert(Graph.Reset() and #Graph.routes == 2)
CheckLimits()
local granted
InventoryFunction = {}
function InventoryFunction.Add(index,count)
    granted = index
    assert(count == 1)
    return true
end
assert(ItemList[9].UpgradeSelectAction() and granted == 9)
print("PASS: automatic directed ports, only merger joins, distributor splits, third connections blocked, atomic reservations and rebuild")
