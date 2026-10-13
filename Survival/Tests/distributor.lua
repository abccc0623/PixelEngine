local root = "D:/PixelEngine/Survival/Asset/"
Grid = {width = 5,height = 5,StartPoint = {x = 1,y = 3},EndPoint = {x = 5,y = 3}}
for x = 1, 5 do
    Grid[x] = {}
    for y = 1, 5 do Grid[x][y] = {ItemIndex = 0,Position = {x = x * 20,y = y * 20,z = 0}} end
end
Grid[1][3].ItemIndex = 2
Grid[2][3].ItemIndex = 5
Grid[5][3].ItemIndex = 3
for _, point in ipairs({{2,2},{3,2},{4,2},{2,4},{3,4},{4,4},{4,3}}) do Grid[point[1]][point[2]].ItemIndex = 1 end
local transforms, arrivals, disabled, id = {}, {}, {}, 0
local paused = false
Time = {}
function Time.IsPaused()
    return paused
end
GameFlow = {State = "Initializing"}
function GameFlow.OnEnergyArrived(attack)
    arrivals[#arrivals + 1] = attack
end
function PVector3(x,y,z)
    return {x = x,y = y,z = z}
end
Transform = {}
function Transform.Get(targetID)
    return transforms[targetID]
end
Pool = {}
function Pool.SetExpansionSize()


end
function Pool.Active()
    id = id + 1
    transforms[id] = {}
    return id
end
function Pool.Disable(name,targetID)
    disabled[targetID] = true
end
dofile(root .. "Table/Graph.lua")
dofile(root .. "Table/EnergyFunction.lua")
assert(Graph.Reset())
assert(#Graph.routes == 2)
assert(Grid[3][2].RoadTexture ~= "road_3" and Grid[3][4].RoadTexture ~= "road_3")
GameFlow.State = "Combat"
local exits = {}
for resource = 1, 4 do
    assert(EnergyFunction.Spawn(1,resource))
    local targetID = id
    EnergyFunction.Update(0.75)
    local item = EnergyFunction.Items[targetID]
    assert(item and item.Attack == resource and item.Progress == 30)
    assert(item.Path[3].y == (resource % 2 == 1 and 2 or 4))
    assert(transforms[targetID].position.y == (resource % 2 == 1 and 50 or 70))
    exits[#exits + 1] = item.Path[3].y
    while EnergyFunction.Items[targetID] do EnergyFunction.Update(0.1) end
end
assert(id == 4 and #arrivals == 4)
for i = 1, 4 do assert(arrivals[i] == i and disabled[i]) end

-- Concurrent traffic on both routes must keep spacing after merging.
arrivals = {}
local spawned = 0
for frame = 1, 1200 do
    if spawned < 12 and EnergyFunction.Spawn(1,7) then spawned = spawned + 1 end
    EnergyFunction.Update(0.025)
    local distances = EnergyFunction.GetDistances(Graph.pathNodes)
    for _, item in pairs(EnergyFunction.Items) do
        local limit = EnergyFunction.FrontLimit(item,EnergyFunction.GetDistances(item.Path))
        assert(limit >= item.Progress - 0.001, "Resources overlap on shared route")
    end
end
assert(spawned == 12 and #arrivals == 12 and next(EnergyFunction.Items) == nil)

-- Editing on the secondary branch must not restart or discard that resource.
EnergyFunction.Distributors["2:3"] = 2
assert(EnergyFunction.Spawn(1,9))
local targetID = id
EnergyFunction.Update(1)
assert(EnergyFunction.Items[targetID].Path[3].y == 4)
local before = transforms[targetID].position
local attack = EnergyFunction.Items[targetID].Attack
EnergyFunction.SaveBeforeEdit()
GameFlow.State = "Build"
paused = true
assert(Graph.Reset())
EnergyFunction.RestoreAfterEdit()
assert(EnergyFunction.Items[targetID].Attack == attack)
assert(transforms[targetID].position.x == before.x and transforms[targetID].position.y == before.y)
local progress = EnergyFunction.Items[targetID].Progress
EnergyFunction.Update(1)
assert(EnergyFunction.Items[targetID].Progress == progress)
paused = false
GameFlow.State = "Combat"
for frame = 1, 100 do EnergyFunction.Update(0.1) end
assert(disabled[targetID])
print("PASS: A/B/A/B, no delay or duplication, unchanged damage, both roads, merge spacing, edit restore")
