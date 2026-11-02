return function(build)
	local rebuilds = 0
	PVector2 = function(x,y)
		return {x = x,y = y}
	end
	Grid = {width = 1,height = 1,BoardID = 7,[1] = {[1] = {ItemIndex = 1,Script = {Input = PVector2(-1,0),OutPut = PVector2(1,0)}}}}
	Grid.RebuildRoadGroups = function()
		rebuilds = rebuilds + 1
	end
	local instance = setmetatable({},{__index = build})
	Entity = {GetScript = function(id)
		assert(id == 7)
		return instance
	end}
	assert(User.Tools[2].Name == "회전")
	User.Tools[2].Action(1,1)
	local road = Grid[1][1].Script
	assert(road.Input.x == 0 and road.Input.y == -1)
	assert(road.OutPut.x == 0 and road.OutPut.y == 1)
	for i = 1, 3 do
		assert(instance:Rotate(1,1))
	end
	assert(road.Input.x == -1 and road.OutPut.x == 1)
	assert(not instance:Rotate(0,1))
	instance.IsLocked = true
	assert(not instance:Rotate(1,1))
	instance.IsLocked = false
	Grid[1][1].Script = nil
	assert(not instance:Rotate(1,1))
	Grid[1][1] = {ItemIndex = 2,Script = {Direction = 270,SetDirection = function(self,angle)
		self.Direction = angle % 360
	end}}
	assert(instance:Rotate(1,1))
	assert(Grid[1][1].Script.Direction == 0 and rebuilds == 5)
end
