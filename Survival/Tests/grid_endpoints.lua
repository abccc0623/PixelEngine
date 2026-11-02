local root = "D:/PixelEngine/Survival/Asset/"
dofile(root .. "Table/ItemList.lua")
dofile(root .. "Script/Grid/Grid.lua")
dofile(root .. "Script/Grid/GridFunction.lua")
local resets, refunds = 0,0
EnergyFunction = {}
function EnergyFunction.StopPreview()


end
Graph = {}
function Graph.Reset()
    resets = resets + 1
end
Debug = {}
function Debug.Log()


end
InventoryFunction = {}
function InventoryFunction.Add()
    refunds = refunds + 1
    return true
end
assert(GridFunction.Add(6,1,2))
assert(GridFunction.Add(6,12,3))
for y = 2, 11 do assert(GridFunction.Add(6,y,1)) end
assert(Grid.StartPoint.x == 6 and Grid.EndPoint.x == 6)
for _, y in ipairs({1,12}) do
    local index = Grid[6][y].ItemIndex
    local before = resets
    for item = 1, #ItemList do assert(not GridFunction.Add(6,y,item)) end
    assert(not GridFunction.Remove(6,y))
    assert(GridFunction.IsLocked(6,y))
    assert(Grid[6][y].ItemIndex == index and Grid[6][y].ItemRotation == 0)
    assert(resets == before and refunds == 0)
end
assert(not GridFunction.Add(5,1,2) and not GridFunction.Add(5,12,3))
assert(Grid[5][1].ItemIndex == 0 and Grid[5][12].ItemIndex == 0)
assert(not GridFunction.IsLocked(6,2))
assert(GridFunction.Add(6,2,7) and refunds == 1)
assert(GridFunction.Remove(6,2) and refunds == 2)
assert(Grid[6][2].ItemIndex == 0)
assert(not GridFunction.Remove(0,0))
print("PASS: centered endpoints, protected overwrite/delete/relocation, normal cells editable")

local script = dofile(root .. "Script/Grid/GridEntity.pxm")
script.Editing, script.SelectTr, script.SelectW, script.SelectH = true,{},6,1
function script:HandleMoveKey()


end
local completed = false
function script:Hide()
    completed = true
end
GameFlow = {State = "Build"}
function GameFlow.CompleteBuild()
    completed = true
end
EnergyFunction = {}
function EnergyFunction.Update()


end
Time = {}
function Time.GetUnscaledDeltaTime()
    return 0
end
Keycode = {X = 1,Z = 2,Escape = 3}
Input = {}
function Input.Key()
    return false
end
function Input.KeyDown(key)
    return key == Keycode.Z
end
InventoryType = {Tool = 1,Inventory = 2,Complete = 3}
User = {Selection = {Type = 1,Index = 1},Tools = {{}}}
User.Tools[1].Action = function(x,y)
    Grid[x][y].ItemRotation = 90
end
for _, y in ipairs({1,12}) do
    script.SelectH = y
    script:Update(0)
    assert(Grid[6][y].ItemRotation == 0,"Locked endpoint received a tool action")
end
User.Selection.Type = InventoryType.Complete
User.Selection.Index = 2
script:Update(0)
assert(completed,"Complete must still work while cursor is on endpoint")
print("PASS: locked cells reject tool actions including rotation; completion remains available")
