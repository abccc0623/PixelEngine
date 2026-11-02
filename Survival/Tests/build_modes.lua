-- 관련 모듈을 미리 로드한 환경에서 실행하는 빌드 모드 회귀 검사.
return function(prototypes)
	local scripts,transforms,active,events = {},{},{},{}
	local nextID = 0
	local paused = false
	local unscaled = 0.25
	local previewReceived = 0
	local function Noop()


	end
	PVector2 = function(x,y)
		return {x = x,y = y}
	end
	PVector3 = function(x,y,z)
		return {x = x,y = y,z = z}
	end
	local function Widget()
		return setmetatable({}, {__index = function()
			return Noop
		end})
	end
	Transform = {Add = function(id)
		local transform = Widget()
		transform.position = PVector3(0,0,0)
		transform.rotation = PVector3(0,0,0)
		transforms[id] = transform
		return transform
	end,Get = function(id)
		return transforms[id]
	end}
	UIImage = {Add = Widget}
	UIText = {Add = Widget}
	Animation2D = {Add = Widget}
	ItemVisual = {Apply = Noop}
	EnergyList = {{}}
	EnergyList.Play = function()
		return true
	end
	ItemList = {{Input = {x = -1,y = 0},OutPut = {x = 1,y = 0}},{Texture = "item_01"},{Texture = "item_00"}}
	Time = {SetPaused = function(value)
		paused = value
	end,IsPaused = function()
		return paused
	end,GetUnscaledDeltaTime = function()
		return unscaled
	end}
	Entity = {}
	function Entity.CreateEmpty()
		nextID = nextID + 1
		active[nextID] = true
		return nextID
	end
	function Entity.Create(name)
		local id = Entity.CreateEmpty()
		local script = setmetatable({ID = id},{__index = assert(prototypes[name])})
		scripts[id] = script
		script:Spawn()
		return id
	end
	function Entity.GetScript(id)
		return scripts[id]
	end
	function Entity.SetActive(id,value)
		assert(active[id] ~= nil)
		active[id] = value
	end
	function Entity.Destroy(id)
		scripts[id],transforms[id],active[id] = nil,nil,nil
	end
	Event = {Bind = function(id,name,method)
		events[name] = events[name] or {}
		table.insert(events[name],{ID = id,Method = method})
	end,Call = function(name)
		for _, callback in ipairs(events[name] or {}) do
			local script = assert(scripts[callback.ID])
			script[callback.Method](script)
		end
	end}
	PlayerManager = {CheckLevelUp = Noop}
	UpgradeListFunction = {Choice = function()
		return true
	end}
	Grid.width,Grid.height,Grid.BoardID = 12,12,nil
	Grid.RoadGroups = {}
	RoadTransport.Resources = {}
	local build = scripts[Entity.Create("BuildSystem")]
	build:Add(2,1,2)
	Grid[2][1].Script:SetDirection(90)
	build:Add(2,2,1)
	Grid[2][2].Script.Input = PVector2(0,-1)
	Grid[2][2].Script.OutPut = PVector2(0,1)
	build:Add(2,3,3)
	Grid[2][3].Script:SetDirection(90)
	Grid[2][3].Script.OnReceive = function()
		if build.IsEditing then
			previewReceived = previewReceived + 1
		end
	end
	Grid.RebuildRoadGroups()
	assert(prototypes.BasicInput.Update == nil and prototypes.BasicOutput.Update == nil)
	GameFlow.Init()
	GameFlow.StartCombat()
	assert(build.IsLocked and not build:Add(3,3,1))
	build:Update(0.5)
	assert(#RoadTransport.Resources == 0)
	build:Update(0.5)
	local real = assert(RoadTransport.Resources[1])
	local originalX,originalY = real.X,real.Y
	local originalHead = Grid.RoadGroups[1].HeadDistance
	local originalElapsed = Grid[2][1].Script.Elapsed
	local _,side = RoadTransport.input(2,2,1,1,2)
	assert(side and side.Entry)
	GameFlow.BeginUpgrade(1)
	assert(paused and build.IsEditing and not active[real.ID])
	assert(#RoadTransport.Resources == 0 and build.SavedResources[1] == real)
	build:Update(0)
	assert(#RoadTransport.Resources == 0)
	assert(not GameFlow.TestBuild())
	assert(GameFlow.Choose(1))
	assert(GameFlow.TestBuild() and build.IsTesting)
	for frame = 1, 40 do
		build:Update(0)
	end
	assert(paused and previewReceived > 0)
	assert(real.X == originalX and real.Y == originalY and scripts[real.ID] == real.Script)
	GameFlow.CompleteBuild()
	assert(GameFlow.State == "Closing" and not build.IsTesting and #RoadTransport.Resources == 0)
	GameFlow.OnBookClosed()
	assert(not paused and build.IsLocked and not build.IsEditing)
	assert(RoadTransport.Resources[1] == real and active[real.ID])
	assert(Grid.RoadGroups[1].HeadDistance == originalHead)
	assert(Grid[2][1].Script.Elapsed == originalElapsed)
	assert(real.X == originalX and real.Y == originalY)
	build:Update(0.1)
	assert(real.Y > originalY)
	-- 다른 길을 추가해 그룹이 재생성되어도 옆에서 진입 중인 실제 자원 위치를 유지한다.
	local sideX,sideY = side.X,side.Y
	GameFlow.BeginUpgrade(1)
	GameFlow.Choose(1)
	assert(build:Add(3,3,1))
	GameFlow.CompleteBuild()
	GameFlow.OnBookClosed()
	assert(side.X == sideX and side.Y == sideY and side.Entry)
	-- 수정하면 테스트는 종료되고, 삭제된 길의 실제 자원은 사라지지 않는다.
	GameFlow.BeginUpgrade(1)
	GameFlow.Choose(1)
	GameFlow.TestBuild()
	for frame = 1, 4 do
		build:Update(0)
	end
	assert(build:Add(2,2,-1) and not build.IsTesting and #RoadTransport.Resources == 0)
	GameFlow.CompleteBuild()
	GameFlow.OnBookClosed()
	assert(RoadTransport.Resources[1] == real and real.Group == nil and active[real.ID])
	-- 테스트/완료 버튼은 인벤토리 선택에서 Z로만 실행하며 플레이 중에는 무시한다.
	UI = {BookID = 1}
	User = {Selection = {Type = 3,Index = 1}}
	InventoryType = {Tool = 1,Inventory = 2,Complete = 3}
	Keycode = {Z = 1}
	Input = {KeyDown = function(key)
		return key == Keycode.Z
	end}
	local inventory = setmetatable({UIList = {}},{__index = prototypes.InventoryEntity})
	inventory:CreateComplete()
	assert(#User.CompleteComponent == 2)
	inventory:Update()
	assert(not build.IsTesting)
	GameFlow.BeginUpgrade(1)
	GameFlow.Choose(1)
	inventory:Update()
	assert(build.IsTesting and paused)
	User.Selection.Index = 2
	inventory:Update()
	assert(GameFlow.State == "Closing" and not build.IsTesting and paused)
end
