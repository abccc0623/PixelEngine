-- LevelUpBook 프로토타입을 전달받아 엔진 호출을 대체한 환경에서 검사합니다.
return function(prototype,pagePrototype)
	local active = true
	local activeIDs,scripts,transforms = {},{},{}
	local images = {}
	local nextID = 100
	local pressed = nil
	local runningTweens = {}
	local moves = {}
	TweenProperty = {PositionY = 1,ScaleX = 6,ScaleY = 7}
	Tween = {PingPong = function(id,property,minimum,maximum,duration,unscaled)
		assert(minimum == 1 and maximum == 1.04 and duration == 0.45 and unscaled)
		runningTweens[id .. ":" .. property] = true
		return true
	end,Move = function(id,property,startValue,endValue,duration,unscaled)
		assert(property == TweenProperty.PositionY and duration == 0.12 and unscaled)
		local move = {ID = id,Start = startValue,End = endValue}
		moves[#moves + 1] = move
		runningTweens[id .. ":" .. property] = move
		return true
	end,Kill = function(id,property)
		runningTweens[id .. ":" .. property] = nil
		return true
	end}
	local animations,loops,callbacks = {},{},{}
	local animation = {}
	function animation:Create(name,frames,rows,speed)
		local id = #animations + 1
		animations[id] = {Name = name,Frames = frames}
		return id
	end
	function animation:SetUseUnscaledTime(value)
		self.Unscaled = value
	end
	function animation:SetLoop(id,value)
		loops[id] = value
	end
	function animation:SetFrameCallback(id,frame,repeated)
		callbacks[id] = {Frame = frame,Repeated = repeated}
	end
	function animation:Play(id)
		self.Current = id
		self.Stopped = false
	end
	function animation:Reset()
		self.Frame = 0
	end
	function animation:Stop()
		self.Stopped = true
	end
	local image = {}
	function image:SetOrder(value)
		self.Order = value
	end
	function image:SetTextureSize(width,height)
		self.Width,self.Height = width,height
	end
	function image:SetPivot(x,y)
		self.PivotX,self.PivotY = x,y
	end
	PVector3 = function(x,y,z)
		return {x = x,y = y,z = z}
	end
	Transform = {Add = function(id)
		local transform = {position = PVector3(0,0,0)}
		function transform:SetParent(parentID)
			self.ParentID = parentID
		end
		transforms[id] = transform
		return transform
	end}
	local function Widget()
		local widget = {}
		function widget:SetText(content)
			self.Content = content
		end
		function widget:SetColor(r,g,b,a)
			self.Color = {r,g,b,a}
		end
		function widget:SetTexture(texture)
			self.Texture = texture
		end
		return setmetatable(widget,{__index = function()
			return function()


			end
		end})
	end
	UIImage = {Add = function(id)
		if id == 51 then
			return image
		end
		images[id] = Widget()
		return images[id]
	end}
	UIText = {Add = Widget}
	ItemVisual = {Apply = function(target,item)
		target.Visual = item
	end}
	Keycode = {Up = 1,Down = 2,Z = 3,X = 4}
	Input = {KeyDown = function(key)
		return pressed == key
	end}
	Animation2D = {Add = function()
		return animation
	end}
	Entity = {SetActive = function(id,value)
		activeIDs[id] = value
		if id == 51 then
			active = value
		end
		for childID, transform in pairs(transforms) do
			if transform.ParentID == id then
				Entity.SetActive(childID,value)
			end
		end
	end}
	function Entity.CreateEmpty()
		nextID = nextID + 1
		activeIDs[nextID] = true
		return nextID
	end
	function Entity.Create(name)
		local id = Entity.CreateEmpty()
		local definition = pagePrototype
		if name == "LevelUpBuildPage" then
			-- 책 전환 검사는 두 번째 페이지의 내부 기능과 분리한다.
			definition = {HandlesInput = true}
			function definition:Spawn()
				self.transform = Transform.Add(self.ID)
			end
			function definition:Setting(bookID)
				self.transform:SetParent(bookID)
			end
			function definition:Show()
				Entity.SetActive(self.ID,true)
			end
			function definition:Hide()
				Entity.SetActive(self.ID,false)
			end
		else
			assert(name == "LevelUpChoicePage")
		end
		local page = setmetatable({ID = id},{__index = definition})
		scripts[id] = page
		page:Spawn()
		return id
	end
	function Entity.GetScript(id)
		return scripts[id]
	end
	local book = setmetatable({ID = 51},{__index = prototype})
	scripts[51] = book
	book:Spawn()
	assert(not active and book.State == "closed" and animation.Unscaled)
	for _, name in ipairs({"open","next","close"}) do
		local id = book.animationList[name]
		assert(loops[id] == false and callbacks[id].Frame == animations[id].Frames - 1)
	end
	assert(book:Show() and active and book.State == "opening")
	assert(not activeIDs[book.Pages[1].ID] and not activeIDs[book.Pages[2].ID])
	assert(not book:Show() and not book:Next())
	book:OnAnimationCallBack(99,book.animationList.open,7)
	assert(book.State == "opening")
	book:OnAnimationCallBack(51,book.animationList.open,7)
	assert(book.State == "idle" and animation.Stopped and image.Width == 768)
	local choice = book.Pages[1].Script
	local firstSlot,secondSlot = choice.Slots[1],choice.Slots[2]
	assert(runningTweens[firstSlot.ID .. ":6"] and runningTweens[firstSlot.ID .. ":7"])
	firstSlot.Transform.scale = PVector3(1.02,1.02,1)
	assert(#choice.Slots == 3 and activeIDs[choice.ID] and not activeIDs[book.Pages[2].ID])
	assert(choice.DescriptionLines[1].Content:find("마법책에",1,true))
	local ruledLines = 0
	for id, widget in pairs(images) do
		if widget.Texture == "line_3" then
			ruledLines = ruledLines + 1
			assert(transforms[id].position.x == 116.5)
			local y = transforms[id].position.y
			assert(y == 0 or y == 40 or y == 80)
			assert(transforms[id].scale.x == 0.75 and transforms[id].scale.y == 0.75)
		end
	end
	assert(ruledLines == 3 and #choice.DescriptionLines == 6)
	pressed = Keycode.Down
	choice:Update()
	assert(choice.SelectedIndex == 2 and choice.Slots[2].Name.Content == ItemList[2].Name)
	assert(moves[#moves].ID == choice.SelectID and moves[#moves].Start == -84 and moves[#moves].End == -9)
	assert(not runningTweens[firstSlot.ID .. ":6"] and not runningTweens[firstSlot.ID .. ":7"])
	assert(firstSlot.Transform.scale.x == 1 and firstSlot.Transform.scale.y == 1)
	assert(runningTweens[secondSlot.ID .. ":6"] and runningTweens[secondSlot.ID .. ":7"])
	assert(choice.DescriptionLines[1].Content:find("자원을",1,true))
	assert(choice.Slots[2].Name.Color[1] == 160 and choice.Slots[1].Name.Color[1] == 37)
	assert(choice.Slots[2].Icon.Visual.FrameCount == 8)
	-- 이동이 끝나기 전에 선택을 바꿔도 도착점이 아닌 현재 좌표에서 시작한다.
	choice.SelectTransform.position.y = -50
	pressed = Keycode.Up
	choice:Update()
	assert(moves[#moves].Start == -50 and moves[#moves].End == -84)
	pressed = Keycode.Down
	choice:Update()
	assert(moves[#moves].Start == -50 and moves[#moves].End == -9)
	pressed = Keycode.Z
	choice:Update()
	pressed = nil
	assert(book.State == "turning" and book.SelectedItemIndex == 2 and book.PageIndex == 1)
	assert(next(runningTweens) == nil)
	assert(not activeIDs[choice.ID] and not activeIDs[book.Pages[2].ID])
	book:OnAnimationCallBack(51,book.animationList.open,7)
	assert(book.State == "turning")
	book:OnAnimationCallBack(51,book.animationList.next,8)
	assert(book.State == "idle" and active and book.PageIndex == 2)
	assert(not activeIDs[choice.ID] and activeIDs[book.Pages[2].ID])
	assert(not book:Next())
	book:AddPage()
	assert(book:Next())
	book:OnAnimationCallBack(51,book.animationList.next,8)
	assert(book.PageIndex == 3 and activeIDs[book.Pages[3].ID] and not activeIDs[book.Pages[2].ID])
	assert(book:Close() and book.State == "closing")
	book:OnAnimationCallBack(51,book.animationList.close,4)
	assert(book.State == "closed" and not active and animation.Stopped)
	assert(book:Show() and active)
	book:OnAnimationCallBack(51,book.animationList.open,7)
	assert(book.PageIndex == 1 and book.SelectedItemIndex == nil and choice.SelectedIndex == 1)
	assert(choice.SelectTransform.position.y == -84 and not runningTweens[choice.SelectID .. ":1"])
	pressed = Keycode.X
	book:Update()
	assert(book.State == "closing" and not activeIDs[choice.ID])
	assert(next(runningTweens) == nil)
end
