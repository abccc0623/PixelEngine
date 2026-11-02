local root = "D:/PixelEngine/Survival/Asset/"
dofile(root .. "Table/MonsterList.lua")
dofile(root .. "Table/WaveList.lua")
dofile(root .. "Script/Manager/GameTimeManager.lua")
dofile(root .. "Script/Manager/MonsterManager.lua")
local paused = false
Time = {}
function Time.IsPaused()
    return paused
end
Entity = {}
function Entity.Create(name)
    assert(name == "GameTimeText")
end
local created = {}
function MonsterManager.Create(name,distance)
    assert(MonsterList[name] and distance == 20)
    created[name] = (created[name] or 0) + 1
end
local function Reset()
    created = {}
    paused = false
    MonsterManager.Init()
    GameTimeManager.Init()
end
local function Advance(seconds)
    GameTimeManager.Update(seconds)
    MonsterManager.Update(seconds)
end

Reset()
Advance(150)
assert(created.Bat == 50 and created.Skeleton == 21)
MonsterManager.Update(0)
assert(created.Bat == 50 and created.Skeleton == 21,"Same time spawned duplicates")
Advance(150)
assert(created.Bat == 100 and created.Skeleton == 42)
assert(GameTimeManager.CurrentWave == 2 and GameTimeManager.WaveTime == 0)
Advance(150)
assert(created.Skeleton == 192)
Advance(150)
assert(created.Skeleton == 342 and GameTimeManager.CurrentWave == 3)
Advance(100)
assert(created.Bat == 100 and created.Skeleton == 342)
assert(WaveList[1].monsters[1].spawned == nil,"Spawn counters leaked into planning data")

Reset()
Advance(450)
assert(created.Bat == 100 and created.Skeleton == 192)
assert(GameTimeManager.CurrentWave == 2 and GameTimeManager.WaveTime == 150)
Reset()
Advance(700)
assert(created.Bat == 100 and created.Skeleton == 342)
assert(GameTimeManager.CurrentWave == 3 and GameTimeManager.WaveTime == 100)
Reset()
paused = true
GameTimeManager.WaveTime = 150
MonsterManager.Update(150)
assert(next(created) == nil)
paused = false
MonsterManager.Update(0)
assert(created.Bat == 50 and created.Skeleton == 21)
Reset()
Advance(150)
assert(created.Bat == 50 and created.Skeleton == 21,"Restart retained old spawn counts")

local saved = WaveList[1].monsters[1].name
WaveList[1].monsters[1].name = "Missing"
assert(not pcall(MonsterManager.Init))
WaveList[1].monsters[1].name = saved
local count = WaveList[1].monsters[1].count
WaveList[1].monsters[1].count = 0.5
assert(not pcall(MonsterManager.Init))
WaveList[1].monsters[1].count = count

EnemyEffects = {}
function EnemyEffects.Reset(monster)
    monster.Effects = {}
end
for name, definition in pairs(MonsterList) do
    local monster = dofile(root .. "Script/Entity/Monster/" .. name .. ".pxm")
    monster.animation = {}
    function monster.animation:Play(index)
        assert(index == 0)
    end
    monster.animationTabel = {move = 0}
    monster.MonsterState = {Move = 0}
    monster:ActiveSetting()
    assert(monster.HP == definition.Stats.HP and monster.Speed == definition.Stats.Speed)
    assert(monster.EXP == definition.Stats.EXP)
    monster.HP, monster.Speed = 0,0
    definition.Stats.Damage = 2
    monster:ActiveSetting()
    assert(monster.HP == definition.Stats.HP and monster.Speed == definition.Stats.Speed)
    assert(monster.Damage == 2,"Added stats did not reach monster instance")
    assert(definition.Stats.HP > 0,"Instance damage changed shared stats")
    definition.Stats.Damage = nil
end
print("PASS: monster stats, pooled reset, extra stat fields, five-minute totals, restart, pause, boundaries and large deltas")
