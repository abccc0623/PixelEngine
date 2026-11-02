-- 현재 BuildSystem 선택 좌표와 두 번째 페이지 입력 연결을 검사합니다.
return function(buildPrototype,pagePrototype)
	local active,tweens,transforms = {},{},{}
	local nextID = 10
	local pressed,heldX,heldArrow = nil,false,nil
	PVector3 = function(x,y,z)
		return {x = x,y = y,z = z}
	end
	local function Widget()
		return setmetatable({}, {__index = function()
			return function()


			end
		end})
	end
	Transform = {Add = function(id)
		local transform = Widget()
		transforms[id] = transform
		return transform
	end}
	UIImage = {Add = Widget}
	Entity = {CreateEmpty = function()
		nextID = nextID + 1
		return nextID
	end,SetActive = function(id,value)
		active[id] = value
	end}
	TweenProperty = {ScaleX = 6,ScaleY = 7}
	Tween = {PingPong = function(id,property,minimum,maximum,duration,unscaled)
		assert(minimum == 1 and maximum == 1.1 and duration == 0.5 and unscaled)
		tweens[id .. ":" .. property] = true
	end,Kill = function(id,property)
		tweens[id .. ":" .. property] = nil
	end}
	Grid = {width = 12,height = 12}
	for x = 1, 12 do
		Grid[x] = {}
		for y = 1, 12 do
			Grid[x][y] = {ItemIndex = -1}
		end
	end
	local build = setmetatable({ID = 1,IsEditing = true,IsLocked = false},{__index = buildPrototype})
	build:CreateSelection()
	assert(not active[build.SelectionID])
	build:ShowSelection(true)
	assert(active[build.SelectionID] and tweens[build.SelectionID .. ":6"] and tweens[build.SelectionID .. ":7"])
	assert(build.SelectionTransform.position.x == -110 and build.SelectionTransform.position.y == -110)
	assert(build:MoveSelection(12,12))
	assert(build.SelectionTransform.position.x == 110 and build.SelectionTransform.position.y == 110)
	assert(not build:MoveSelection(13,12) and build.SelectX == 12)
	build:MoveSelection(1,1)
	local book = {State = "idle",PageIndex = 2}
	Entity.GetScript = function(id)
		return book
	end
	Keycode = {Up = 1,Down = 2,Left = 3,Right = 4,X = 5,Z = 6}
	Time = {GetUnscaledDeltaTime = function()
		return 0.1
	end}
	Input = {KeyDown = function(key)
		return key == pressed
	end,Key = function(key)
		return (key == Keycode.X and heldX) or key == heldArrow
	end}
	InventoryType = {Tool = 1,Inventory = 2,Complete = 3}
	local toolCalls,placements,adds = 0,0,0
	local allowAdd = true
	User = {Selection = {Type = InventoryType.Inventory,Index = 1},Inventory = {{ItemIndex = 1,ItemCount = 5}},Tools = {{Action = function(x,y)
		assert(x == build.SelectX and y == build.SelectY)
		toolCalls = toolCalls + 1
	end}}}
	InventoryFunction = {Place = function(index)
		placements = placements + 1
	end}
	function build:Add(x,y,index)
		adds = adds + 1
		return allowAdd
	end
	local page = setmetatable({IsShown = true,BookID = 99,Build = build,KeyRepeatStates = {}},{__index = pagePrototype})
	pressed = Keycode.Right
	page:Update()
	assert(build.SelectX == 2 and build.SelectY == 1)
	pressed,heldX = Keycode.Down,true
	page:Update()
	assert(build.SelectX == 2 and build.SelectY == 1)
	pressed,heldX = Keycode.Down,false
	page:Update()
	assert(build.SelectY == 2)
	pressed,heldArrow = nil,Keycode.Right
	for frame = 1, 7 do
		page:Update()
	end
	assert(build.SelectX > 2)
	pressed,heldArrow = Keycode.Z,nil
	page:Update()
	assert(adds == 1 and placements == 1)
	allowAdd = false
	page:Update()
	assert(adds == 2 and placements == 1)
	Grid[build.SelectX][build.SelectY].ItemIndex = 1
	page:Update()
	assert(adds == 2 and placements == 1)
	User.Selection.Type = InventoryType.Tool
	page:Update()
	assert(toolCalls == 1)
	User.Selection.Type = InventoryType.Complete
	page:Update()
	assert(toolCalls == 1 and adds == 2)
	book.State = "turning"
	pressed = Keycode.Down
	page:Update()
	assert(build.SelectY == 2)
	build:HideSelection()
	assert(not active[build.SelectionID] and next(tweens) == nil)
	assert(build.SelectionTransform.scale.x == 1)
	build.IsEditing = false
	assert(not build:ShowSelection(true))
end
