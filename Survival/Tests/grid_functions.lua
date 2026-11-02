Direction = {None = 0,Up = 1,Down = 2,Left = 3,Right = 4}
ItemList = {}
Event = {Call = function()


end}
Grid = {width = 12,height = 12}
for x = 1,12 do
	Grid[x] = {}
	for y = 1,12 do
		Grid[x][y] = {ItemIndex = 0,ItemRotation = 90}
	end
end
local returned = {}
InventoryFunction = {}
function InventoryFunction.Add(index,count)
	returned[#returned + 1] = {index = index,count = count}
	return false
end
dofile("Asset/Script/Grid/GridFunction.lua")
assert(GridFunction.Add(1,1,2))
assert(Grid[1][1].Direction == Direction.Down)
assert(GridFunction.Add(1,2,1))
assert(Grid[1][2].Direction == Direction.Down and Grid[1][2].ItemRotation == 90)
assert(GridFunction.Add(1,1,999))
assert(#returned == 1 and returned[1].index == 2 and returned[1].count == 1)
assert(GridFunction.Get(1,1) == 999 and Grid[1][1].ItemRotation == 0)
assert(GridFunction.Remove(1,1) and GridFunction.Get(1,1) == 0)
assert(GridFunction.Remove(1,1))
assert(GridFunction.Add(12,12,3) and GridFunction.Get(12,12) == 3)
for _, xy in ipairs({{0,1},{13,1},{1,0},{1,13},{1.5,1},{1,1.5},{"1",1},{false,1},{math.huge,1},{0/0,1}}) do
	assert(not GridFunction.Add(xy[1],xy[2],1))
	assert(not GridFunction.Remove(xy[1],xy[2]))
	assert(GridFunction.Get(xy[1],xy[2]) == 0)
end
assert(not GridFunction.Add(nil,1,1))
assert(GridFunction.Get(1,nil) == 0)
for _, index in ipairs({-1,1.5,"1",false,math.huge,0/0}) do
	assert(not GridFunction.Add(1,1,index))
end
assert(not GridFunction.Add(1,1,nil))
assert(GridFunction.Add(1,1,0))
Grid[2] = nil
assert(not GridFunction.Add(2,1,1) and GridFunction.Get(2,1) == 0)
Grid[3][3].Direction = 4
assert(GridFunction.Add(3,3,1) and Grid[3][3].Direction == 0)
Grid[3][3].Direction = 2
assert(GridFunction.Remove(3,3) and Grid[3][3].Direction == 0)
for _, case in ipairs({{6,5,Direction.Down,90},{6,7,Direction.Up,270},{5,6,Direction.Right,0},{7,6,Direction.Left,180}}) do
	for x = 1,12 do
		Grid[x] = {}
		for y = 1,12 do
			Grid[x][y] = {ItemIndex = 0,Direction = 0,ItemRotation = 0}
		end
	end
	Grid[case[1]][case[2]] = {ItemIndex = 1,Direction = case[3]}
	Grid[6][6].ItemIndex = 1
	assert(GridFunction.SetRotation(6,6))
	assert(Grid[6][6].Direction == case[3] and Grid[6][6].ItemRotation == case[4])
end
Grid[6][5] = {ItemIndex = 1,Direction = Direction.Down}
assert(GridFunction.SetRotation(6,6) and Grid[6][6].Direction == 0 and Grid[6][6].ItemRotation == 0)
Grid[6][5].Direction = Direction.Up
Grid[7][6].Direction = Direction.Right
assert(GridFunction.SetRotation(6,6) and Grid[6][6].Direction == 0)
assert(GridFunction.Add(1,1,1) and Grid[1][1].Direction == 0)
for x = 1,12 do
	for y = 1,12 do
		Grid[x][y] = {ItemIndex = 0,Direction = 0,ItemRotation = 0}
	end
end
assert(GridFunction.Add(6,1,2))
assert(GridFunction.Add(6,2,1))
assert(GridFunction.Add(6,3,1))
assert(GridFunction.Add(7,3,1))
assert(Grid[6][3].Direction == Direction.Right and Grid[6][3].RoadTexture == "road_3" and Grid[6][3].ItemRotation == 180)
assert(Grid[7][3].Direction == Direction.Right)
assert(GridFunction.Remove(7,3))
assert(Grid[6][3].Direction == Direction.Down and Grid[6][3].RoadTexture == "road_1" and Grid[6][3].ItemRotation == 90)
for _, case in ipairs({
	{6,5,Direction.Down,7,6,Direction.Right,0,-1},
	{6,5,Direction.Down,5,6,Direction.Left,180,1},
	{6,7,Direction.Up,7,6,Direction.Right,0,1},
	{6,7,Direction.Up,5,6,Direction.Left,180,-1},
	{5,6,Direction.Right,6,7,Direction.Down,90,1},
	{5,6,Direction.Right,6,5,Direction.Up,270,-1},
	{7,6,Direction.Left,6,7,Direction.Down,90,-1},
	{7,6,Direction.Left,6,5,Direction.Up,270,1},
}) do
	for x = 1,12 do
		for y = 1,12 do
			Grid[x][y] = {ItemIndex = 0,Direction = 0,ItemRotation = 0}
		end
	end
	Grid[case[1]][case[2]] = {ItemIndex = 1,Direction = case[3]}
	Grid[case[4]][case[5]] = {ItemIndex = 1,Direction = case[6]}
	Grid[6][6].ItemIndex = 1
	assert(GridFunction.SetRotation(6,6))
	local texture = "road_2"
	local rotation = case[7]
	if case[8] == -1 then
		texture = "road_3"
		rotation = (rotation + 180) % 360
	end
	assert(Grid[6][6].RoadTexture == texture and Grid[6][6].ItemRotation == rotation)
end
print("grid_functions: PASS")
