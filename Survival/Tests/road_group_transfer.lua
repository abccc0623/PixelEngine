-- RoadTransport를 미리 로드한 테스트 환경에서 반환 함수를 실행한다. 엔진 실행용 파일이 아니다.
return function()
	local created,destroyed = 0,0
	local scripts = {}
	PVector2 = function(x,y)
		return {x = x,y = y}
	end
	PVector3 = function(x,y,z)
		return {x = x,y = y,z = z}
	end
	EnergyList = {{}}
	Entity = {
		Create = function()
			created = created + 1
			scripts[created] = {transform = {},Setting = function()
				return true
			end}
			return created
		end,
		GetScript = function(id)
			return scripts[id]
		end,
		Destroy = function(id)
			destroyed = destroyed + 1
			scripts[id] = nil
		end,
	}
	local function Setup(reverse)
		Grid = {width = 12,height = 12}
		RoadTransport.Resources = {}
		local source,target = {Tiles = {{x = 5,y = 6}}},{Tiles = {}}
		Grid[5] = {[6] = {ItemIndex = 1,RoadGroup = source,Script = {Input = PVector2(-1,0),OutPut = PVector2(1,0)}}}
		Grid[6] = {}
		for y = 2, 11 do
			target.Tiles[#target.Tiles + 1] = {x = 6,y = y}
			Grid[6][y] = {ItemIndex = 1,RoadGroup = target,Script = {Input = PVector2(0,-1),OutPut = PVector2(0,1)}}
		end
		RoadTransport.BuildPath(source)
		RoadTransport.BuildPath(target)
		Grid.RoadGroups = {source,target}
		if reverse then
			Grid.RoadGroups = {target,source}
		end
		return source,target
	end
	for _, reverse in ipairs({false,true}) do
		local source,target = Setup(reverse)
		local ok,resource = RoadTransport.input(5,6)
		assert(ok)
		assert(resource.X == -40 and resource.Y == -10)
		local allocations = created
		RoadTransport.Update(0)
		assert(#source.Resources == 1)
		RoadTransport.Update(0.1)
		assert(#source.Resources == 1)
		RoadTransport.Update(1)
		assert(#source.Resources == 0 and target.Resources[1] == resource)
		assert(resource.Group == target and resource.TileX == 6 and resource.TileY == 6)
		assert(created == allocations and destroyed == 0 and #RoadTransport.Resources == 1)
		assert(math.abs(target.HeadDistance - 76) < 0.000001)
		assert(resource.X == -22 and resource.Y == -10)
		RoadTransport.Update(0.3)
		assert(math.abs(resource.X + 16) < 0.000001 and resource.Y == -10)
		RoadTransport.Update(0.3)
		assert(resource.X == -10 and resource.Y == -10 and not resource.Entry)
		RoadTransport.Update(20)
		assert(RoadTransport.TakeOutput(6,11) == resource)
		assert(#RoadTransport.Resources == 0)
		destroyed = 0
	end
	local source,target = Setup(false)
	local _,blocker = RoadTransport.input(6,6)
	local _,waiting = RoadTransport.input(5,6)
	source.HeadDistance = source.Length
	source.TailDistance = source.Length
	waiting.Entry = nil
	waiting.X,waiting.Y = RoadTransport.GetPosition(source,source.Length)
	RoadTransport.Speed = 0.000001
	RoadTransport.Update(1)
	assert(source.Resources[1] == waiting and target.Resources[1] == blocker)
	assert(waiting.Group == source and #RoadTransport.Resources == 2)
	RoadTransport.Speed = 20
	RoadTransport.Update(1)
	assert(#source.Resources == 0 and #target.Resources == 2)
	assert(target.Resources[2] == waiting and target.Gaps[1] >= RoadTransport.Spacing)
	source,target = Setup(false)
	RoadTransport.input(5,6)
	Grid[6][6].Script.OutPut = PVector2(-1,0)
	RoadTransport.Update(1)
	assert(#source.Resources == 1 and #target.Resources == 0)
	Grid[6][6] = nil
	RoadTransport.Update(1)
	assert(#source.Resources == 1)
	-- 생성기 방향을 전달하면 상하좌우 모두 맞닿은 경계에서 중앙으로 이동한다.
	for _, side in ipairs({{-1,0},{1,0},{0,-1},{0,1}}) do
		local group = {Tiles = {{x = 6,y = 6}}}
		Grid = {width = 12,height = 12,RoadGroups = {group}}
		Grid[6] = {[6] = {ItemIndex = 1,RoadGroup = group,Script = {Input = PVector2(side[1],side[2]),OutPut = PVector2(-side[1],-side[2])}}}
		RoadTransport.Resources = {}
		RoadTransport.BuildPath(group)
		local ok,resource = RoadTransport.input(6,6,1,6 + side[1],6 + side[2])
		assert(ok and resource.X == -10 + side[1] * 10 and resource.Y == -10 + side[2] * 10)
		RoadTransport.Update(0.25)
		assert(resource.X == -10 + side[1] * 5 and resource.Y == -10 + side[2] * 5)
		RoadTransport.Update(0.25)
		assert(resource.X == -10 and resource.Y == -10)
	end
	source,target = Setup(false)
	local ok,sideEntry = RoadTransport.input(6,6,1,5,6)
	assert(ok and sideEntry.X == -20 and sideEntry.Y == -10)
	RoadTransport.Update(0.25)
	assert(sideEntry.X == -15 and sideEntry.Y == -10)
	RoadTransport.Update(0.25)
	assert(sideEntry.X == -10 and sideEntry.Y == -10)
	RoadTransport.Update(0.25)
	assert(sideEntry.X == -10 and sideEntry.Y == -5)
	assert(not RoadTransport.input(6,6,1,4,6))
	assert(not RoadTransport.input(6,6,1,5.5,5.5))
	-- 순환 경로를 한 바퀴 이상 이동해도 진입 구간이 다시 재생되지 않는다.
	source,target = Setup(false)
	target.IsLoop = true
	RoadTransport.BuildPath(target)
	local _,loopEntry = RoadTransport.input(6,6,1,5,6)
	RoadTransport.Update(target.Length / RoadTransport.Speed + 0.1)
	assert(not loopEntry.Entry)
	-- 두 생성기를 함께 돌려도 자원 중복이나 최소 간격 위반이 없어야 한다.
	source,target = Setup(false)
	local received = 0
	for frame = 1, 1800 do
		if frame % 30 == 1 then
			RoadTransport.input(5,6,1,4,6)
			RoadTransport.input(6,2,1,6,1)
		end
		RoadTransport.Update(1 / 30)
		if RoadTransport.TakeOutput(6,11) then
			received = received + 1
		end
		local seen,count = {},0
		for _, group in ipairs(Grid.RoadGroups) do
			for i, resource in ipairs(group.Resources) do
				assert(not seen[resource.ID] and resource.Group == group)
				seen[resource.ID] = true
				count = count + 1
				if i > 1 then
					assert(group.Gaps[i - 1] >= RoadTransport.Spacing - 0.000001)
				end
			end
		end
		assert(count == #RoadTransport.Resources)
	end
	assert(received > 50)
end
