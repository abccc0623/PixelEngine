dofile("Asset/Script/Grid/Grid.lua")
dofile("Asset/Table/ItemList.lua")
dofile("Asset/Script/Grid/GridGraph.lua")
dofile("Asset/Script/Grid/GridFunction.lua")
Event = {Call = function()


end}
InventoryFunction = {}
function InventoryFunction.Add(index,count)
	return true
end
assert(GridFunction.Add(6,1,2))
assert(GridFunction.Add(6,5,3))
for y = 2,4 do
	assert(GridFunction.Add(6,y,1))
end
assert(GridGraph.IsConnected)
for y = 1,5 do
	assert(GridGraph.Nodes["6:" .. y].OnPath)
end
assert(GridFunction.Remove(6,3))
assert(not GridGraph.IsConnected)
assert(not GridGraph.Nodes["6:2"].OnPath)
Grid.Create()
assert(GridFunction.Add(6,1,2))
assert(GridFunction.Add(3,3,3))
assert(GridFunction.Add(6,2,1))
assert(GridFunction.Add(6,3,5))
assert(GridFunction.Add(5,3,1))
assert(GridFunction.Add(4,3,1))
assert(GridFunction.Add(7,3,1))
assert(GridGraph.IsConnected)
assert(GridGraph.Nodes["6:2"].OnPath)
assert(GridGraph.Nodes["6:3"].OnPath)
assert(GridGraph.Nodes["5:3"].OnPath and GridGraph.Nodes["4:3"].OnPath)
assert(not GridGraph.Nodes["7:3"].OnPath)
assert(#GridGraph.Nodes["6:3"].Next == 2)
Grid[6][3].InputDirection = 0
ItemList[5].InputDirections = {Direction.Down}
assert(not GridGraph.Build())
-- 외부에서 변경한 입구 설정도 다시 생성하면 반영된다.
ItemList[5].InputDirections = {Direction.Up}
assert(GridGraph.Build())
print("grid_graph: PASS")
