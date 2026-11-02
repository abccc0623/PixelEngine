local scripts,transforms,images,children,active = {},{},{},{},{}
local listeners,free,expansion = {},{},{}
local sequence,paused,shots = 0,false,0
local function noop()


end
function PVector3(x,y,z)
	return {x=x,y=y,z=z}
end
Entity = {}
function Entity.CreateEmpty()
	sequence = sequence + 1
	active[sequence] = true
	return sequence
end
function Entity.SetActive(id,value)
	active[id] = value
	for _, child in ipairs(children[id] or {}) do
		Entity.SetActive(child,value)
	end
end
function Entity.CallFunction(id,name,...)
	assert(scripts[id] and scripts[id][name],"Missing callback: " .. tostring(name))
	scripts[id][name](scripts[id],...)
end
Transform = {}
function Transform.Add(id)
	local tr = {position=PVector3(0,0,0),scale=PVector3(1,1,1),rotation=PVector3(0,0,0)}
	function tr:SetParent(parent)
		children[parent] = children[parent] or {}
		children[parent][#children[parent] + 1] = id
	end
	transforms[id] = tr
	return tr
end
function Transform.Get(id)
	return transforms[id]
end
local function Component(id)
	return setmetatable({thisID=id},{__index=function()
		return noop
	end})
end
UIImage = {Get=function(id)
	return images[id]
end}
function UIImage.Add(id)
	local image = Component(id)
	function image:SetColor(r,g,b,a)
		self.alpha = a
	end
	images[id] = image
	return image
end
UIText = {Add=Component}
Animation2D = {Add=function(id)
	local animation = Component(id)
	local nextIndex = 0
	function animation:Create()
		nextIndex = nextIndex + 1
		return nextIndex
	end
	animation.CreateRegion = animation.Create
	return animation
end}
ItemVisual = {Icons={},Apply=noop}
Event = {}
function Event.Bind(id,event,method)
	listeners[event] = listeners[event] or {}
	listeners[event][#listeners[event]+1] = {id,method}
end
function Event.Call(event)
	for _, listener in ipairs(listeners[event] or {}) do
		Entity.CallFunction(listener[1],listener[2])
	end
end
Time = {SetPaused=function(value)
	paused = value
end,IsPaused=function()
	return paused
end,GetUnscaledDeltaTime=function()
	return 0.1
end}
Engine = {BackgroundColor=noop}
Debug = {LogWarning=noop,LogError=error}
Input = {Key=function()
	return false
end,KeyDown=function()
	return false
end}
Keycode = {X=1,Z=2,Up=3,Down=4,Left=5,Right=6,Escape=7}
UI = {BookID=-1}
local function UIElement()
	local id = Entity.CreateEmpty()
	Transform.Add(id)
	return id
end
UI.CreateBookTitleIcon = UIElement
UI.CreateBookTitleText = UIElement
UI.CreateBookSubText = UIElement
Pool = {SetExpansionSize=function(name,count)
	expansion[name] = count
end}
function Pool.Active(name)
	free[name] = free[name] or {}
	if #free[name] == 0 then
		for i=1,expansion[name] or 1 do
			free[name][#free[name]+1] = Entity.Create(name)
		end
	end
	local id = table.remove(free[name])
	Entity.SetActive(id,true)
	return id
end
function Pool.Disable(name,id)
	Entity.SetActive(id,false)
	free[name][#free[name]+1] = id
end
BulletManager = {Create=function(attack,index)
	assert(attack == 1 and index == 1)
	shots = shots + 1
end}
dofile("Asset/Script/Grid/Grid.lua")
dofile("Asset/Table/ItemList.lua")
dofile("Asset/Table/User.lua")
dofile("Asset/Table/UpgradeList.lua")
dofile("Asset/Table/UpgradeListFunction.lua")
dofile("Asset/Table/InventoryFunction.lua")
dofile("Asset/Table/EnergyList.lua")
dofile("Asset/Script/Manager/PlayerManager.lua")
dofile("Asset/Script/Manager/GameFlow.lua")
dofile("Asset/Script/Grid/GridGraph.lua")
dofile("Asset/Script/Grid/GridFunction.lua")
local byName = {}
function Entity.Create(name)
	assert(name ~= "GridResources" and name ~= "EnergyItem","Removed movement must not create resources")
	local id = Entity.CreateEmpty()
	if name == "GameLogic" then
		PlayerManager.transform = {}
		PlayerManager.ExpSlider = Component(id)
		return id
	end
	local path = "Asset/Script/Grid/" .. name .. ".pxm"
	if name == "Book" or name == "InventoryEntity" then
		path = "Asset/Script/Entity/UI/" .. name .. ".pxm"
	elseif name == "Content" then
		return id
	end
	local script = dofile(path)
	script.ID = id
	scripts[id] = script
	byName[name] = script
	script:Spawn()
	return id
end
dofile("Asset/Scene/InGame.scene"):Start()
assert(GameFlow.State == "Combat" and not paused and GridGraph.IsConnected)
assert(Grid[6][1].ItemIndex == 2 and Grid[6][12].ItemIndex == 3)
for y=2,11 do
	assert(Grid[6][y].ItemIndex == 1 and Grid[6][y].Direction == Direction.Down)
end
assert(#ItemList == 5 and ItemList[5].Texture == "router")
assert(User.Inventory[1].ItemCount == 30)
assert(transforms[Grid.BoardID].position.x == 1760)
local interaction,book = byName.GridInteraction,byName.Book
assert(byName.GridResources == nil and byName.EnergyItem == nil and GameFlow.OnEnergyArrived == nil)
GameFlow.BeginUpgrade(1)
assert(paused and not Grid.Visible and GameFlow.State == "Choice")
book:OnAnimationCallBack(book.ID,book.animationList.open,7)
UpgradeList[1].ItemIndex = 1
assert(GameFlow.Choose(1))
book:OnAnimationCallBack(book.ID,book.animationList.next,8)
assert(GameFlow.State == "Build" and interaction.Editing and Grid.Visible)
assert(User.Selection.Type == InventoryType.Tool and User.Selection.Index == 1)
assert(transforms[Grid.BoardID].scale.x == 2.4 or math.abs(transforms[Grid.BoardID].scale.x-2.4)<0.00001)
User.Selection = {Type=InventoryType.Inventory,Index=1}
interaction:Move(4,4)
interaction:Action()
assert(GridFunction.Get(4,4) == 1 and User.Inventory[1].ItemCount == 49)
User.Selection = {Type=InventoryType.Tool,Index=1}
interaction:Action()
assert(GridFunction.Get(4,4) == 0 and User.Inventory[1].ItemCount == 50)
interaction:Move(6,1)
interaction:Action()
assert(GridFunction.Get(6,1) == 2)
assert(interaction.Test == nil and #User.CompleteComponent == 1)
GridFunction.Add(4,4,1)
assert(GameFlow.State == "Build" and shots == 0)
User.Selection = {Type=InventoryType.Complete,Index=1}
interaction:Action()
assert(GameFlow.State == "Closing")
book:OnAnimationCallBack(book.ID,book.animationList.close,4)
assert(GameFlow.State == "Combat" and not paused and not interaction.Editing)
assert(Grid.Visible and transforms[Grid.BoardID].scale.x == 1)
assert(shots == 0)
print("grid_ingame: PASS")
