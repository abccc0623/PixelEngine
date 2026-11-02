local root = "D:/PixelEngine/Survival/Asset/"
local nextID, paused, attacks, completions = 0,false,0,0
local transforms, images, labels, scripts, active, pools, events = {},{},{},{},{},{},{}
local function Noop()


end
function PVector3(x,y,z)
    return {x = x,y = y,z = z}
end
local function Widget()
    return setmetatable({}, {__index = function()
        return Noop
    end})
end
Transform = {}
function Transform.Add(id)
    local tr = {position = PVector3(0,0,0),scale = PVector3(1,1,1),rotation = PVector3(0,0,0)}
    function tr:SetParent(parent)
        self.Parent = parent
    end
    transforms[id] = tr
    return tr
end
function Transform.Get(id)
    return transforms[id]
end
UIImage = {}
function UIImage.Add(id)
    local image = Widget()
    image.thisID = id
    function image:SetColor(r,g,b,a)
        self.Alpha = a
    end
    images[id] = image
    return image
end
function UIImage.Get(id)
    return images[id]
end
UIText = {}
function UIText.Add(id)
    local text = Widget()
    function text:SetText(value)
        labels[id] = value
    end
    return text
end
Animation2D = {}
function Animation2D.Add()
    local animation = Widget()
    function animation:CreateRegion()
        return 0
    end
    return animation
end
Entity = {}
function Entity.CreateEmpty()
    nextID = nextID + 1
    active[nextID] = true
    return nextID
end
function Entity.SetActive(id,value)
    active[id] = value
end
function Entity.CallFunction(id,name,...)
    return scripts[id][name](scripts[id],...)
end
Event = {}
function Event.Bind(id,name,method)
    events[name] = events[name] or {}
    table.insert(events[name],{ID = id,Method = method})
end
function Event.Call(name)
    for _, callback in ipairs(events[name] or {}) do Entity.CallFunction(callback.ID,callback.Method) end
end
Time = {}
function Time.SetPaused(value)
    paused = value
end
function Time.IsPaused()
    return paused
end
function Time.GetDeltaTime()
    return paused and 0 or 0.025
end
function Time.GetUnscaledDeltaTime()
    return 0.025
end
Input = {Key = function()
    return false
end, KeyDown = function()
    return false
end}
Keycode = {Up = 1,Down = 2,Left = 3,Right = 4,X = 5,Z = 6,Escape = 7}
Pool = {SetExpansionSize = Noop}
function Pool.Active(name)
    local id = Entity.CreateEmpty()
    pools[id] = name
    local script = dofile(root .. "Script/Grid/EnergyItem.pxm")
    scripts[id], script.ID = script,id
    script:Spawn()
    return id
end
function Pool.Disable(name,id)
    active[id] = false
end
local function PoolIDs(name)
    local ids = {}
    for id, pool in pairs(pools) do
        if pool == name and active[id] then ids[#ids + 1] = id end
    end
    table.sort(ids)
    return ids
end
function Pool.GetActiveCount(name)
    return #PoolIDs(name)
end
function Pool.GetActiveID(name,index)
    return PoolIDs(name)[index + 1]
end
UI = {BookID = Entity.CreateEmpty(),CreateBookSubText = Entity.CreateEmpty,CreateBookTitleIcon = Entity.CreateEmpty,CreateBookTitleText = Entity.CreateEmpty}
Transform.Add(UI.BookID)
Debug = {Log = Noop,LogError = Noop,LogWarning = Noop}
ItemVisual = {Apply = Noop}
PlayerManager = {CheckLevelUp = Noop,Attack = function()
    attacks = attacks + 1
end}
UpgradeListFunction = {Choice = function()
    return true
end}
dofile(root .. "Table/User.lua")
dofile(root .. "Table/ItemList.lua")
dofile(root .. "Table/EnergyList.lua")
dofile(root .. "Script/Grid/Grid.lua")
dofile(root .. "Table/InventoryFunction.lua")
dofile(root .. "Table/EnergyFunction.lua")
dofile(root .. "Script/Manager/GameFlow.lua")
Graph = {SetRoadImage = Noop}
function Graph.Reset()
    Graph.pathNodes = {}
    for y = 1, 12 do Graph.pathNodes[y] = {x = 6,y = y} end
    Graph.IsConnected, Graph.routes = true,{Graph.pathNodes}
end
function Graph.SpawnEnergy()
    return EnergyFunction.Spawn(1,1)
end
Graph.Reset()
dofile(root .. "Script/Grid/GridFunction.lua")
for y = 1, 12 do Grid[6][y].ItemIndex = 1 end
Grid[6][1].ItemIndex, Grid[6][12].ItemIndex = 2,3
Grid.StartPoint, Grid.EndPoint = {x = 6,y = 1},{x = 6,y = 12}
Grid[6][2].ItemIndex, Grid[6][3].ItemIndex, Grid[6][4].ItemIndex = 7,8,6
local function CreateScript(path)
    local script = dofile(root .. path)
    local id = Entity.CreateEmpty()
    scripts[id], script.ID = script,id
    script:Spawn()
    return script
end
GameFlow.Init()
local board = CreateScript("Script/Grid/GridEntity.pxm")
local inventory = CreateScript("Script/Entity/UI/InventoryEntity.pxm")
assert(#User.CompleteComponent == 2)
assert(User.CompleteComponent[1].TransformComponent.position.x < User.CompleteComponent[2].TransformComponent.position.x)
local names = {}
for _, value in pairs(labels) do names[value] = true end
assert(names["테스트"] and names["완료"])
GameFlow.StartCombat()
assert(EnergyFunction.Spawn(1,8))
EnergyFunction.Update(0.1)
local combatID = next(EnergyFunction.Items)
local original = EnergyFunction.Items[combatID]
local progress = original.Progress
InventoryFunction.Move(InventoryType.Inventory,3)
GameFlow.BeginUpgrade()
assert(User.Selection.Type == InventoryType.Tool and User.Selection.Index == 1)
Event.Call("HideBookContent")
assert(GameFlow.Choose(1))
Event.Call("ShowGrid")
Event.Call("ShowInventory")
scripts[combatID]:Update()
assert(images[combatID].Alpha == 0 and not EnergyFunction.Preview)
local pressed, modifier = nil,false
function Input.Key(key)
    return modifier and key == Keycode.X
end
function Input.KeyDown(key)
    return key == pressed
end
InventoryFunction.Move(InventoryType.Inventory,12)
modifier, pressed = true,Keycode.Down
inventory:Update()
assert(User.Selection.Type == InventoryType.Complete and User.Selection.Index == 1)
pressed = Keycode.Right
inventory:Update()
assert(User.Selection.Index == 2)
pressed = Keycode.Left
inventory:Update()
assert(User.Selection.Index == 1)
pressed = Keycode.Up
inventory:Update()
assert(User.Selection.Type == InventoryType.Inventory and User.Selection.Index == 12)
pressed = Keycode.Down
inventory:Update()
modifier, pressed = false,Keycode.Z
board:Update(0)
pressed = nil
assert(EnergyFunction.Preview and paused)
local previewID = next(EnergyFunction.Items)
assert(previewID ~= combatID and EnergyFunction.IsVisible(previewID) and not EnergyFunction.IsVisible(combatID))
local processed = false
for frame = 1, 700 do
    board:Update(0)
    for id in pairs(EnergyFunction.Items) do
        local item = EnergyFunction.Items[id]
        scripts[id]:Update()
        if item.SplitCount > 0 then
            assert(item.Attack == 0.5 and item.ChainCount == 1 and item.ExplosionRadius == 1)
            processed = true
        end
    end
end
assert(processed and attacks == 0 and original.Progress == progress)
assert(GridFunction.Add(6,5,7))
assert(not EnergyFunction.Preview and EnergyFunction.Items[combatID] == original)
assert(not EnergyFunction.IsVisible(combatID))
board:Test()
assert(EnergyFunction.Preview)
assert(not GridFunction.Remove(6,1) and EnergyFunction.Preview,"Rejected edit stopped preview")
assert(GridFunction.Remove(6,5) and not EnergyFunction.Preview)
GridFunction.Add(6,5,1)
board:Test()
assert(EnergyFunction.Preview)
board:Hide()
assert(not EnergyFunction.Preview)
GameFlow.CompleteBuild()
GameFlow.OnBookClosed()
assert(GameFlow.State == "Combat" and not paused)
assert(EnergyFunction.Items[combatID] == original and original.Progress == progress)
assert(EnergyFunction.IsVisible(combatID))
InventoryFunction.Move(InventoryType.Inventory,5)
GameFlow.BeginUpgrade()
assert(User.Selection.Type == InventoryType.Tool and User.Selection.Index == 1)
print("PASS: button layout, selection reset, hidden edit resources, paused real-time preview, processing/split, no attacks, edit cancellation and preserved combat resources")
