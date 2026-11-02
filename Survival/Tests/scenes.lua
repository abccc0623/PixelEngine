local root = "D:/PixelEngine/Survival/Asset/"
local imports, created, calls, selectedScene = {}, {}, {}, nil
Asset = {}
function Asset.Import(path)
    imports[#imports + 1] = path
end
Scene = {}
function Scene.Change(name)
    selectedScene = name
end
Entity = {}
function Entity.Create(name)
    created[#created + 1] = name
    return #created
end
GameFlow = {}
function GameFlow.Init()
    calls[#calls + 1] = "Init"
end
function GameFlow.StartCombat()
    assert(created[#created] == "GridInteraction")
    calls[#calls + 1] = "Combat"
end
Engine = {}
function Engine.SetWindowSize(width,height)
    assert(width == 1920 and height == 1080)
end
function Engine.BackgroundColor(r,g,b)
    assert(r == 20 and g == 27 and b == 38)
end
PlayerManager = {}
function PlayerManager.AddExp(value)
    error("Startup must not grant experience")
end
InventoryFunction = {}
local roads = 0
function InventoryFunction.Add(index,count)
    assert(index == 1 and count == 30)
    assert(created[#created] == "InventoryEntity")
    roads = roads + count
    return true
end
Grid = {}
function Grid.Create()
end
GridFunction = {}
function GridFunction.Create()
	Entity.Create("GridEntity")
	Entity.Create("GridInteraction")
end
local cells = {}
function GridFunction.Add(x,y,item)
    cells[#cells + 1] = {x = x,y = y,item = item}
    if item == 2 then Grid.StartPoint = {x = x,y = y} end
    if item == 3 then Grid.EndPoint = {x = x,y = y} end
end
dofile(root .. "main.lua")
Main()
assert(selectedScene == "GridTest" and #imports == 5)
local title = dofile(root .. "Scene/Title.scene")
title:Start()
assert(#created == 0 and #calls == 0)
local game = dofile(root .. "Scene/InGame.scene")
game:Start()
assert(calls[1] == "Init" and calls[2] == "Combat")
assert(#calls == 2 and roads == 30)
assert(table.concat(created,",") == "Book,Content,InventoryEntity,GameLogic,GridEntity,GridInteraction")
assert(#cells == 12 and Grid.StartPoint.x == 6 and Grid.EndPoint.x == 6 and Grid.StartPoint.y == 1 and Grid.EndPoint.y == 12)
for i = 3, #cells do assert(cells[i].x == 6 and cells[i].y == i - 1 and cells[i].item == 1) end
print("PASS: startup selects InGame; centered endpoints and initial path")
