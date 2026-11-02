-- Run from D:/PixelEngine/Survival with Lua 5.1 / LuaJIT.
local root = "D:/PixelEngine/Survival/Asset/"
local paused, events, attacks = false, {}, {}
local attackIndices = {}
local attackElements = {}
local attackRadii = {}
Entity = {}
function Entity.CallFunction(id,name,itemIndex)
    assert(name == "Setting" and itemIndex >= 1)
end
EnergyFunction = {}
function EnergyFunction.SaveBeforeEdit()


end
function EnergyFunction.RestoreAfterEdit()


end
Time = {}
function Time.SetPaused(value)
    paused = value
end
function Time.IsPaused()
    return paused
end
Event = {}
function Event.Call(name)
    events[#events + 1] = name
end
UpgradeListFunction = {}
function UpgradeListFunction.Choice(index)
    return index == 1
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
BulletManager = {}
function BulletManager.Create(attack,itemIndex,element,chainCount,explosionRadius)
    attacks[#attacks + 1] = attack
    attackIndices[#attacks] = itemIndex
    attackElements[#attacks] = element
    attackRadii[#attacks] = explosionRadius
end
PlayerManager.transform = {}
GameFlow.Init()
GameFlow.StartCombat()
assert(GameFlow.State == "Combat" and not paused and events[#events] == "ShowGridPlay")
GameFlow.OnEnergyArrived(0.5,2,"Fire",2,3)
assert(#attacks == 1 and attacks[1] == 0.5)
assert(attackIndices[1] == 2)
assert(attackElements[1] == "Fire")
assert(attackRadii[1] == 3)
PlayerManager.AddExp(3)
assert(GameFlow.State == "Choice" and paused and PlayerManager.Level == 2)
assert(events[#events] == "BookOpen")
GameFlow.OnEnergyArrived(9)
assert(#attacks == 1)
assert(not GameFlow.Choose(2) and GameFlow.State == "Choice")
assert(GameFlow.Choose(1) and GameFlow.State == "Build" and paused)
assert(events[#events] == "BookNext")
GameFlow.CompleteBuild()
assert(GameFlow.State == "Closing" and paused and events[#events] == "BookClose")
GameFlow.OnBookClosed()
assert(GameFlow.State == "Combat" and not paused)
-- Excess EXP grants all choices before a single Grid session.
PlayerManager.AddExp(10)
assert(PlayerManager.Level == 4 and GameFlow.State == "Choice" and GameFlow.PendingChoices == 2)
GameFlow.Choose(1)
assert(GameFlow.State == "Choice" and GameFlow.PendingChoices == 1 and paused)
PlayerManager.Exp = 0
GameFlow.Choose(1)
assert(GameFlow.State == "Build" and GameFlow.PendingChoices == 0)
GameFlow.CompleteBuild()
GameFlow.OnBookClosed()

function PVector3(x,y,z)
    return {x = x,y = y,z = z}
end
Grid = {[1] = {{ItemIndex = 2,Position = PVector3(0,0,0)}, {ItemIndex = 1,Position = PVector3(20,0,0)}, {ItemIndex = 3,Position = PVector3(40,0,0)}}}
Graph = {IsConnected = true,pathNodes = {{x = 1,y = 1},{x = 1,y = 2},{x = 1,y = 3}}}
function Graph.GetBranches(path)
    return {path}
end
local transforms, disabled, id = {}, {}, 0
Transform = {}
function Transform.Get(targetID)
    return transforms[targetID]
end
Pool = {}
function Pool.SetExpansionSize()


end
function Pool.Active()
    id = id + 1
    transforms[id] = {}
    return id
end
function Pool.Disable(name,targetID)
    disabled[targetID] = true
end
dofile(root .. "Table/EnergyFunction.lua")
assert(EnergyFunction.Spawn(1,0.25,0,2,"Electric"))
Time.SetPaused(true)
EnergyFunction.Update(10)
assert(EnergyFunction.Items[1].Progress == 0 and not EnergyFunction.Spawn(1,1))
Time.SetPaused(false)
local count = #attacks
EnergyFunction.Update(2)
assert(#attacks == count + 1 and attacks[#attacks] == 0.25 and disabled[1])
assert(attackIndices[#attacks] == 2)
assert(attackElements[#attacks] == "Electric")
EnergyFunction.Update(2)
assert(#attacks == count + 1)
-- With one connected outlet the distributor forwards one unchanged resource.
Grid[1][2].ItemIndex = 5
assert(EnergyFunction.Spawn(1,8))
count = #attacks
for frame = 1, 100 do EnergyFunction.Update(0.1) end
assert(#attacks == count + 1 and attacks[count + 1] == 8)
assert(next(EnergyFunction.Items) == nil)

-- Projectile payload reaches both supported enemy types.
dofile(root .. "Script/Manager/BulletManager.lua")
BulletManager.Init()
function BulletManager.GetPoolMinDistanceTargetID()
    return 77
end
transforms[77] = {position = PVector3(1,0,0)}
PlayerManager.transform.position = {}
function PlayerManager.transform.position:Direction()
    return PVector3(1,0,0)
end
local configuredID
Entity = {}
function Entity.CallFunction(targetID,name,direction,itemIndex,element)
    if name == "Setting" then return end
    if itemIndex == nil then assert(name == "ActiveSetting") return end
    assert(name == "ActiveSetting" and direction.x == 1)
    assert(itemIndex == 2)
    assert(element == "Ice")
    configuredID = targetID
end
Rigidbody2D = {}
function Rigidbody2D.Get()
    return {SetPosition = function()


    end}
end
BulletManager.Create(0.25,2,"Ice")
assert(configuredID and BulletManager.GetAttack(configuredID) == 0.25)
BulletManager.Attacks[99] = 0.5
ffi = {}
function ffi.string(value)
    return value
end
Rigidbody2D = {}
function Rigidbody2D.Get()
    return {GetLayer = function()
        return "Bullet"
    end}
end
dofile(root .. "Table/AttributeList.lua")
dofile(root .. "Table/EnemyEffects.lua")
for index, name in ipairs({"Bat","Skeleton"}) do
    local enemy = dofile(root .. "Script/Entity/Monster/" .. name .. ".pxm")
    enemy.ID = 200 + index
    enemy.transform = {position = PVector3(0,0,0)}
    BulletManager.Hits[99] = nil
    enemy.HP = 3
    enemy.MonsterState = {Hit = 2,Death = 1}
    enemy:OnCollisionEnter(99)
    assert(enemy.HP == 2.5)
end
print("PASS: combat/choice/build/closing, pause, queued levels, energy recycling, distributor payload, enemy damage")

-- Editing must preserve in-flight resources and queue orphaned resources.
local received = {}
local onArrival = GameFlow.OnEnergyArrived
function GameFlow.OnEnergyArrived(attack,itemIndex)
    received[#received + 1] = attack
    attackIndices[#received] = itemIndex
end
local oldPath = Graph.pathNodes
Grid[1][2].ItemIndex = 1
assert(EnergyFunction.Spawn(1,6,2))
local movingID = id
EnergyFunction.Update(0.75)
assert(EnergyFunction.Items[movingID].Progress == 30)
EnergyFunction.SaveBeforeEdit()
Graph.pathNodes = {{x = 1,y = 1},{x = 1,y = 2},{x = 1,y = 3}}
EnergyFunction.RestoreAfterEdit()
local moving = EnergyFunction.Items[movingID]
assert(moving.Progress == 30 and moving.Attack == 6 and moving.SplitCount == 2)
assert(moving.Path == Graph.pathNodes and transforms[movingID].position.x == 30)
EnergyFunction.Update(1)
assert(EnergyFunction.Items[movingID] == nil)

Grid[1][2].ItemIndex = 5
assert(EnergyFunction.Spawn(1,8,0,2))
local distributorID = id
EnergyFunction.Update(0.5)
assert(EnergyFunction.Items[distributorID].Progress == 20)
assert(EnergyFunction.Items[distributorID].Attack == 8)
EnergyFunction.SaveBeforeEdit()
Graph.pathNodes = {{x = 1,y = 1},{x = 1,y = 2},{x = 1,y = 3}}
EnergyFunction.RestoreAfterEdit()
assert(EnergyFunction.Items[distributorID].Progress == 20)
EnergyFunction.SaveBeforeEdit()
-- Remove the occupied distributor cell from the path.
Grid[1][2].ItemIndex = 0
Graph.IsConnected = false
Graph.pathNodes = {}
EnergyFunction.RestoreAfterEdit()
assert(next(EnergyFunction.Items) == nil and #EnergyFunction.AttackQueue == 1)
received = {}
Time.SetPaused(true)
EnergyFunction.Update(5)
assert(#received == 0 and #EnergyFunction.AttackQueue == 1)
Time.SetPaused(false)
EnergyFunction.Update(0.01)
assert(#received == 1 and received[1] == 8)
assert(attackIndices[1] == 2)
EnergyFunction.QueueAttack({Attack = 3})
EnergyFunction.Update(0.1)
assert(#received == 1)
EnergyFunction.Update(0.1)
assert(#received == 2 and received[2] == 3 and #EnergyFunction.AttackQueue == 0)
GameFlow.OnEnergyArrived = onArrival
Graph.pathNodes = oldPath
Graph.IsConnected = true
print("PASS: edit preserves position and damage; disconnected queue respects pause and interval")

-- Exercise the actual Book callbacks: startup hidden, pause until close ends.
local book = dofile(root .. "Script/Entity/UI/Book.pxm")
local active, playing = {}, nil
Entity = {}
function Entity.SetActive(targetID,value)
    active[targetID] = value
end
function Transform.Add()
    return {}
end
UIImage = {}
function UIImage.Add()
    return setmetatable({}, {__index = function()
        return function()


        end
    end})
end
Animation2D = {}
function Animation2D.Add()
    local animation = UIImage.Add()
    local animationID = 0
    function animation:Create()
        animationID = animationID + 1
        return animationID
    end
    function animation:Play(value)
        playing = value
    end
    return animation
end
function Event.Bind()


end
UI = {}
book.ID = 100
book:Spawn()
assert(active[100] == false and playing == book.animationList.idle)
GameFlow.BeginUpgrade()
book:BookOpen()
assert(active[100] and paused)
book:OnAnimationCallBack(100,book.animationList.open,7)
assert(events[#events] == "ShowRandomCard")
GameFlow.Choose(1)
book:BookNext()
book:OnAnimationCallBack(100,book.animationList.next,8)
assert(events[#events] == "ShowInventory" and paused)
GameFlow.CompleteBuild()
book:BookClose()
assert(paused)
book:OnAnimationCallBack(100,book.animationList.close,4)
assert(not paused and GameFlow.State == "Combat" and not active[100])
print("PASS: actual Book startup and open/next/close callbacks")
