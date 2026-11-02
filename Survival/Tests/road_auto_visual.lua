return function(roadPrototype,buildPrototype)
	PVector2 = function(x,y)
		return {x = x,y = y}
	end
	Grid.width,Grid.height = 3,3
	Grid.BoardID = nil
	for x = 1, 3 do
		Grid[x] = {}
		for y = 1, 3 do
			Grid[x][y] = {ItemIndex = -1}
		end
	end
	RoadTransport = {BuildPath = function(group)
		group.Length = #group.Tiles * 20
	end,Rebind = function()

	end}
	local road = setmetatable({Input = PVector2(-1,0),OutPut = PVector2(1,0),transform = {rotation = {}}},{__index = roadPrototype})
	road.ui_image = {SetTexture = function(self,name)
		self.Texture = name
	end,SetTiling = function()

	end,SetOffset = function()

	end,SetTextureSize = function()

	end}
	Grid[2][2] = {ItemIndex = 1,ID = 20,Script = road}
	Grid[1][2] = {ItemIndex = 2,ID = 10,Script = {OutPut = PVector2(1,0)}}
	Grid.RebuildRoadGroups()
	assert(road.TextureName == "road_1")
	local build = setmetatable({},{__index = buildPrototype})
	assert(build:Rotate(2,2))
	assert(road.OutPut.x == 0 and road.OutPut.y == 1)
	assert(road.Input.x == -1 and road.Input.y == 0)
	assert(road.TextureName == "road_2" and road.transform.rotation.z == 90)
	assert(build:Rotate(2,2))
	assert(road.TextureName == "road_1" and road.Input.x == 1)
	Grid[1][2].Script.OutPut = PVector2(0,-1)
	Grid[2][3] = {ItemIndex = 2,Script = {OutPut = PVector2(0,-1)}}
	road.OutPut = PVector2(1,0)
	Grid.RebuildRoadGroups()
	assert(road.TextureName == "road_2" and road.transform.rotation.z == 0)
	Grid[2][3].Script = nil
	Grid[2][1] = {ItemIndex = 2,Script = {OutPut = PVector2(0,1)}}
	Grid.RebuildRoadGroups()
	assert(road.TextureName == "road_3" and road.transform.rotation.z == 180)
	Grid[2][1].Script = nil
	Grid.RebuildRoadGroups()
	assert(road.TextureName == "road_1" and road.Input.x == -1)
end
