local root = "D:/PixelEngine/Survival/Asset/"
dofile(root .. "Table/ItemList.lua")
local transforms, arrivals, disabled, nextID = {}, {}, {}, 0
local paused = false
Time = {}
function Time.IsPaused()
    return paused
end
function PVector3(x,y,z)
    return {x = x,y = y,z = z}
end
Transform = {}
function Transform.Get(id)
    return transforms[id]
end
Pool = {}
function Pool.SetExpansionSize()


end
function Pool.Active()
    nextID = nextID + 1
    transforms[nextID] = {}
    return nextID
end
function Pool.Disable(name,id)
    disabled[id] = true
end
Entity = {}
function Entity.CallFunction(id,name,index)
    assert(name == "Setting" and index == 2)
end
GameFlow = {State = "Combat"}
function GameFlow.OnEnergyArrived(attack,index,element,chainCount,explosionRadius)
    arrivals[#arrivals + 1] = {Attack = attack,ItemIndex = index,Element = element,ChainCount = chainCount,ExplosionRadius = explosionRadius}
end
local function Reset(twoSplitters)
    Grid = {[1] = {}}
    local path = {}
    for y = 1, 5 do
        Grid[1][y] = {ItemIndex = 1,Position = PVector3((y - 1) * 20,0,0)}
        path[y] = {x = 1,y = y}
    end
    Grid[1][1].ItemIndex = 2
    Grid[1][5].ItemIndex = 3
    Grid[1][4].ItemIndex = 6 -- Penultimate cell, the historical crash scenario.
    if twoSplitters then Grid[1][2].ItemIndex = 6 end
    Graph = {IsConnected = true,pathNodes = path,routes = {path}}
    dofile(root .. "Table/EnergyFunction.lua")
    arrivals = {}
    paused = false
end
local function CheckSpacing()
    for id, item in pairs(EnergyFunction.Items) do
        for otherID in pairs(EnergyFunction.Items) do
            if otherID ~= id then
                assert(math.abs(transforms[id].position.x - transforms[otherID].position.x) >= EnergyFunction.Gap - 0.001,"Split resources overlapped")
            end
        end
    end
end
local function Drain()
    for frame = 1, 2000 do
        EnergyFunction.Update(0.025)
        CheckSpacing()
        if next(EnergyFunction.Items) == nil and #EnergyFunction.PendingOutputs == 0 and #EnergyFunction.AttackQueue == 0 then return end
    end
    error("Splitter traffic did not drain")
end
Reset(false)
assert(EnergyFunction.Spawn(1,10,0,2))
Drain()
assert(#arrivals == 2 and arrivals[1].Attack == 5 and arrivals[2].Attack == 5)
assert(arrivals[1].ItemIndex == 2 and arrivals[2].ItemIndex == 2)

Reset(true)
local spawned = 0
for frame = 1, 10000 do
    if spawned < 8 and EnergyFunction.Spawn(1,10,0,2) then spawned = spawned + 1 end
    EnergyFunction.Update(0.025)
    CheckSpacing()
    if spawned == 8 and next(EnergyFunction.Items) == nil and #EnergyFunction.PendingOutputs == 0 then break end
end
assert(spawned == 8 and #arrivals == 32,"Each input must split once at each splitter")
local total = 0
for _, arrival in ipairs(arrivals) do
    assert(arrival.Attack == 2.5 and arrival.ItemIndex == 2)
    total = total + arrival.Attack
end
assert(total == 80,"Splitting must preserve total attack")

Reset(false)
assert(EnergyFunction.Spawn(1,10,0,2))
EnergyFunction.Update(3)
assert(#EnergyFunction.PendingOutputs == 1)
paused = true
EnergyFunction.Update(10)
assert(#arrivals == 0 and #EnergyFunction.PendingOutputs == 1)
EnergyFunction.SaveBeforeEdit()
local newPath = {}
for i, node in ipairs(Graph.pathNodes) do newPath[i] = {x = node.x,y = node.y} end
Graph.pathNodes, Graph.routes = newPath,{newPath}
EnergyFunction.RestoreAfterEdit()
assert(#EnergyFunction.PendingOutputs == 1)
paused = false
Drain()
assert(#arrivals == 2 and arrivals[1].Attack == 5 and arrivals[2].Attack == 5)

Reset(false)
assert(EnergyFunction.Spawn(1,10,0,2))
EnergyFunction.Update(3)
EnergyFunction.SaveBeforeEdit()
Graph.IsConnected, Graph.pathNodes, Graph.routes = false,{},{}
EnergyFunction.RestoreAfterEdit()
Drain()
assert(#arrivals == 2 and arrivals[1].Attack + arrivals[2].Attack == 10)
assert(arrivals[1].ItemIndex == 2 and arrivals[2].ItemIndex == 2)
print("PASS: penultimate splitter, half attack, same resource index, repeated splits, traffic spacing, pause/edit and removed-route queue")
Reset(false)
Grid[1][2].ItemIndex = 7
assert(EnergyFunction.Spawn(1,10,0,2))
Drain()
assert(#arrivals == 2 and arrivals[1].Element == "Electric" and arrivals[2].Element == "Electric")
assert(arrivals[1].Attack == 5 and arrivals[2].Attack == 5)
Reset(false)
assert(EnergyFunction.Spawn(1,10,0,2,"Electric"))
EnergyFunction.Update(3)
EnergyFunction.SaveBeforeEdit()
Graph.IsConnected, Graph.pathNodes, Graph.routes = false,{},{}
EnergyFunction.RestoreAfterEdit()
Drain()
assert(#arrivals == 2 and arrivals[1].Element == "Electric" and arrivals[2].Element == "Electric")
print("PASS: last attribute wins; split and edit queue retain attributes")

Reset(false)
Grid[1][4].ItemIndex = 6
assert(EnergyFunction.Spawn(1,12,0,2,"Electric"))
EnergyFunction.Update(1.5)
local processing = EnergyFunction.Items[next(EnergyFunction.Items)]
assert(processing.ProcessingTime == 1.5 and #EnergyFunction.PendingOutputs == 0)
paused = true
EnergyFunction.Update(10)
assert(processing.ProcessingTime == 1.5)
paused = false
EnergyFunction.Update(1.4)
assert(processing.ProcessingTime > 0 and #EnergyFunction.PendingOutputs == 0 and #arrivals == 0)
EnergyFunction.SaveBeforeEdit()
EnergyFunction.RestoreAfterEdit()
assert(processing.ProcessingTime > 0)
EnergyFunction.Update(0.1)
assert(processing.ProcessingTime == nil and #EnergyFunction.PendingOutputs == 1)
Drain()
assert(#arrivals == 2)
for _, arrival in ipairs(arrivals) do
    assert(arrival.Attack == 6 and arrival.ItemIndex == 2 and arrival.Element == "Electric")
end

Reset(false)
Grid[1][4].ItemIndex = 6
assert(EnergyFunction.Spawn(1,12,0,2))
EnergyFunction.Update(5)
Drain()
assert(#arrivals == 2,"Large delta must process the split only once")
print("PASS: two-way split, 1.5-second processing, pause/edit, half attack, attributes and large delta")

for _, index in ipairs({6}) do
    Reset(false)
    Grid[1][2].ItemIndex = 7
    Grid[1][3].ItemIndex = 7
    Grid[1][4].ItemIndex = index
    assert(EnergyFunction.Spawn(1,12,0,2))
    EnergyFunction.Update(4.5)
    local item = EnergyFunction.Items[next(EnergyFunction.Items)]
    assert(item.ProcessingTime == 1.5 and #EnergyFunction.PendingOutputs == 0)
    EnergyFunction.Update(1.4)
    assert(item.ProcessingTime > 0 and #arrivals == 0)
    Drain()
    assert(#arrivals == ItemList[index].SplitAmount)
    for _, arrival in ipairs(arrivals) do
        assert(arrival.Attack == 12 / ItemList[index].SplitAmount and arrival.ChainCount == 2)
    end
end
local added
InventoryFunction = {}
function InventoryFunction.Add(index,count)
    added = index
    assert(count == 1)
    return true
end
for index = 6, 8 do
    ItemList[index].UpgradeSelectAction()
    assert(added == index)
end
assert(#ItemList == 9 and ItemList[6].SplitAmount == 2 and ItemList[6].ProcessTime == 1.5)
print("PASS: single splitter, fixed 1.5 seconds and inherited chain count")

Reset(false)
Grid[1][2].ItemIndex = 7
Grid[1][3].ItemIndex = 7
assert(EnergyFunction.Spawn(1,12,0,2))
EnergyFunction.Update(4)
local resource = EnergyFunction.Items[next(EnergyFunction.Items)]
assert(resource.ChainCount == 2)
EnergyFunction.SaveBeforeEdit()
EnergyFunction.RestoreAfterEdit()
assert(resource.ChainCount == 2)
Drain()
assert(#arrivals == 2 and arrivals[1].ChainCount == 2 and arrivals[2].ChainCount == 2)

Reset(false)
assert(EnergyFunction.Spawn(1,12,0,2))
Drain()
assert(arrivals[1].ChainCount == nil and arrivals[2].ChainCount == nil)
print("PASS: edit preserves accumulated count; no reactor grants no extra bounce")

Reset(false)
Grid[1][2].ItemIndex = 7
Grid[1][3].ItemIndex = 7
assert(EnergyFunction.Spawn(1,12,0,2))
EnergyFunction.Update(6)
EnergyFunction.SaveBeforeEdit()
Graph.IsConnected, Graph.pathNodes, Graph.routes = false,{},{}
EnergyFunction.RestoreAfterEdit()
Drain()
assert(#arrivals == 2 and arrivals[1].ChainCount == 2 and arrivals[2].ChainCount == 2)
print("PASS: disconnected route attack queue retains reactor stacks")

for _, second in ipairs({7,8}) do
    Reset(false)
    Grid[1][2].ItemIndex = 8
    Grid[1][3].ItemIndex = second
    assert(EnergyFunction.Spawn(1,12,0,2))
    EnergyFunction.Update(0.5)
    local resource = EnergyFunction.Items[next(EnergyFunction.Items)]
    assert(resource.ProcessingTime == 1.5 and resource.ExplosionRadius == nil)
    EnergyFunction.Update(1.4)
    assert(resource.ExplosionRadius == nil)
    paused = true
    EnergyFunction.Update(10)
    assert(resource.ExplosionRadius == nil)
    EnergyFunction.SaveBeforeEdit()
    EnergyFunction.RestoreAfterEdit()
    paused = false
    Drain()
    assert(#arrivals == 2)
    for _, arrival in ipairs(arrivals) do
        assert(arrival.Attack == 6 and arrival.ExplosionRadius == (second == 8 and 2 or 1))
        assert(arrival.ChainCount == (second == 7 and 1 or nil))
    end
end
Reset(false)
Grid[1][2].ItemIndex = 8
Grid[1][3].ItemIndex = 7
assert(EnergyFunction.Spawn(1,12,0,2))
EnergyFunction.Update(6)
EnergyFunction.SaveBeforeEdit()
Graph.IsConnected, Graph.pathNodes, Graph.routes = false,{},{}
EnergyFunction.RestoreAfterEdit()
Drain()
assert(#arrivals == 2)
for _, arrival in ipairs(arrivals) do
    assert(arrival.ExplosionRadius == 1 and arrival.ChainCount == 1 and arrival.Attack == 6)
end
print("PASS: timed explosion stacking, chain coexistence, pause/edit, split and disconnected queue")
