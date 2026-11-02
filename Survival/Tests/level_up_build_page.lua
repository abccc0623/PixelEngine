-- 두 번째 페이지와 기존 인벤토리 UI를 연결하는 검사. 모듈은 호출자가 미리 로드한다.
return function(pagePrototype,inventoryPrototype)
	local scripts,transforms,active = {},{},{}
	local nextID,inventoryCreates,buildCreates = 100,0,0
	local paused,pressed,holdX = false,nil,false
	PVector3 = function(x,y,z)
		return {x = x,y = y,z = z}
	end
	local function Widget(id)
		return setmetatable({thisID = id},{__index = function()
			return function()


			end
		end})
	end
	Transform = {}
	function Transform.Add(id)
		local transform = {position = PVector3(0,0,0),scale = PVector3(1,1,1)}
		function transform:SetParent(parentID)
			assert(transforms[parentID])
			self.ParentID = parentID
		end
		function transform:SetChild(childID)
			transforms[childID]:SetParent(id)
		end
		transforms[id] = transform
		return transform
	end
	function Transform.Get(id)
		return transforms[id]
	end
	UIImage = {Add = Widget}
	UIText = {Add = Widget}
	ItemVisual = {Apply = function()


	end}
	Event = {Bind = function()


	end}
	Time = {IsPaused = function()
		return paused
	end,SetPaused = function(value)
		paused = value
	end}
	Entity = {}
	function Entity.CreateEmpty()
		nextID = nextID + 1
		return nextID
	end
	function Entity.GetScript(id)
		return scripts[id]
	end
	function Entity.SetActive(id,value)
		active[id] = value
		for childID, transform in pairs(transforms) do
			if transform.ParentID == id then
				Entity.SetActive(childID,value)
			end
		end
	end
	function Entity.CallFunction(id,name,...)
		local script = assert(scripts[id])
		return script[name](script,...)
	end
	local build = {ID = 9,transform = Transform.Add(9),IsEditing = false}
	build.transform.position = PVector3(960,540,0)
	build.transform.scale = PVector3(4,4,4)
	function build:BeginEdit()
		self.IsEditing = true
	end
	function build:StopTest()
		self.IsTesting = false
	end
	function build:ToggleTest()
		self.IsTesting = not self.IsTesting
	end
	function build:ResumePlay()
		self.IsEditing = false
	end
	function build:ShowSelection(reset)
		self.SelectionShown = true
	end
	function build:HideSelection()
		self.SelectionShown = false
	end
	scripts[9] = build
	Grid = {BoardID = 9,[1] = {[1] = {ItemIndex = 42}}}
	local cell = Grid[1][1]
	function Entity.Create(name)
		if name == "BuildSystem" then
			buildCreates = buildCreates + 1
			Grid.BoardID = 9
			return 9
		end
		assert(name == "InventoryEntity")
		inventoryCreates = inventoryCreates + 1
		local id = Entity.CreateEmpty()
		local inventory = setmetatable({ID = id},{__index = inventoryPrototype})
		scripts[id] = inventory
		inventory:Spawn()
		return id
	end
	local page = setmetatable({ID = 52},{__index = pagePrototype})
	local book = {ID = 51,transform = Transform.Add(51),PageIndex = 2,State = "idle"}
	book.transform.position = PVector3(960,540,1)
	book.transform.scale = PVector3(3,3,1)
	function book:Close()
		self.State = "closing"
		page:Hide()
	end
	scripts[51] = book
	page:Spawn()
	page:Setting(51)
	assert(buildCreates == 0 and inventoryCreates == 1 and Grid[1][1] == cell)
	assert(#User.InventoryComponent == 12 and transforms[page.Inventory.ID].ParentID == page.ID)
	page:OnBookOpened()
	assert(paused and build.IsEditing and not active[9])
	page:Show()
	assert(active[9] and active[page.Inventory.ID])
	assert(build.transform.position.x == 610.5 and build.transform.position.y == 540)
	assert(build.transform.scale.x == 2.25 and build.transform.position.z == 0)
	local count = #page.Inventory.UIList
	page.Inventory:AttachTo(page.ID)
	assert(#page.Inventory.UIList == count)
	Keycode = {X = 1,Z = 2,Down = 3,Right = 4,Left = 5,Up = 6}
	Input = {KeyDown = function(key)
		return key == pressed
	end,Key = function(key)
		return key == Keycode.X and holdX
	end}
	pressed,holdX = Keycode.Down,true
	page.Inventory:Update()
	assert(User.Selection.Type == InventoryType.Inventory and book.State == "idle")
	User.Selection = {Type = InventoryType.Complete,Index = 1}
	pressed,holdX = Keycode.Z,false
	page.Inventory:Update()
	assert(build.IsTesting)
	User.Selection.Index = 2
	page.Inventory:Update()
	assert(book.State == "closing" and not active[9] and not build.IsTesting)
	page:OnBookClosed()
	assert(not paused and not build.IsEditing and active[9])
	assert(build.transform.position.x == 960 and build.transform.scale.x == 4 and Grid[1][1] == cell)
	-- 재사용할 빌드가 없을 때만 생성하고 기존 인벤토리는 다시 만들지 않는다.
	Grid.BoardID = nil
	page:Setting(51)
	assert(buildCreates == 1 and inventoryCreates == 1)
	-- 기존 BookID로 자동 생성되는 인벤토리 경로도 유지한다.
	UI.BookID = 51
	local legacy = scripts[Entity.Create("InventoryEntity")]
	assert(legacy.Initialized and legacy.ParentID == 51 and #User.InventoryComponent == 12)
	assert(not legacy.PageBookID)
end
