local root = "D:/PixelEngine/Survival/Asset/"
local paused, granted, shuffles = false,0,0
local active, events = {},{}
Time = {}
function Time.SetPaused(value)
    paused = value
end
function Time.IsPaused()
    return paused
end
Entity = {}
function Entity.SetActive(id,value)
    active[id] = value
end
Keycode = {Z = 1,Up = 2,Down = 3}
Input = {}
function Input.KeyDown(key)
    return key == Keycode.Z
end
ItemType = {Factory = 2,Stat = 1,System = 3}
local allowed = true
InventoryFunction = {}
function InventoryFunction.Add(index,count)
    if not allowed then return false end
    granted = granted + count
    return true
end
local item = {Name = "test",ItemType = ItemType.Factory,UpgradeContent = "test",UpgradeSelectAction = function()
    return InventoryFunction.Add(1,1)
end}
ItemList = {item,item,item,item}
dofile(root .. "Table/UpgradeList.lua")
dofile(root .. "Table/UpgradeListFunction.lua")
local shuffle = UpgradeListFunction.Shuffle
function UpgradeListFunction.Shuffle()
    shuffles = shuffles + 1
    shuffle()
end
local content = dofile(root .. "Script/Entity/UI/Content.pxm")
content.ID, content.UIList = 10,{11,12}
content.choiceImages, content.descriptionTexts, content.choiceSelectTransform = {},{},{}
function PVector3(x,y,z)
    return {x = x,y = y,z = z}
end
Event = {}
function Event.Call(name)
    events[name] = (events[name] or 0) + 1
    if name == "ShowRandomCard" then content:Show() end
end
dofile(root .. "Script/Manager/GameFlow.lua")
dofile(root .. "Script/Manager/PlayerManager.lua")
PlayerManager.ExpSlider = {}
function PlayerManager.ExpSlider:SetValue(value)
    self.Value = value
end
function PlayerManager.ExpSlider:SetRange(minimum,maximum)
    self.Maximum = maximum
end
GameFlow.Init()
GameFlow.StartCombat()
PlayerManager.AddExp(22) -- 3 + 4 + 6 + 9: four levels.
assert(PlayerManager.Level == 5 and PlayerManager.Exp == 0 and PlayerManager.ExpMax == 13)
assert(GameFlow.PendingChoices == 4 and GameFlow.State == "Choice" and paused)
assert(events.BookOpen == 1)
Event.Call("ShowRandomCard") -- Book open animation completed.
assert(shuffles == 1 and active[10])
allowed = false
content:Update()
assert(GameFlow.PendingChoices == 4 and granted == 0 and shuffles == 1 and active[10])
allowed = true
for remaining = 3, 0, -1 do
    content.selectIndex = 4
    content:Update()
    assert(GameFlow.PendingChoices == remaining)
    if remaining > 0 then
        assert(GameFlow.State == "Choice" and active[10] and active[11] and paused)
        assert(content.selectIndex == 1 and not events.BookNext)
    end
end
assert(granted == 4 and shuffles == 4)
assert(GameFlow.State == "Build" and not active[10] and events.BookNext == 1)
assert(not GameFlow.Choose(1) and granted == 4)
GameFlow.CompleteBuild()
GameFlow.OnBookClosed()
assert(GameFlow.State == "Combat" and not paused and events.BookOpen == 1)
assert(events.BookClose == 1)

-- EXP arriving while the choice page is already open joins the same batch.
PlayerManager.AddExp(13)
assert(GameFlow.PendingChoices == 1 and PlayerManager.Level == 6)
local opens = events.BookOpen
PlayerManager.AddExp(19)
assert(GameFlow.PendingChoices == 2 and PlayerManager.Level == 7)
assert(events.BookOpen == opens)
Event.Call("ShowRandomCard")
content:Update()
assert(GameFlow.State == "Choice" and GameFlow.PendingChoices == 1)
content:Update()
assert(GameFlow.State == "Build" and GameFlow.PendingChoices == 0)
print("PASS: four-level batch, per-choice shuffle/reset, failed choice preserved, single Grid transition, no reopen and late EXP aggregation")
