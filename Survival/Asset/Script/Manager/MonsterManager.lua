MonsterManager = {Spawned = {}}

function MonsterManager.ApplyStats(monster,name)
    local definition = assert(MonsterList[name],"Unknown monster: " .. tostring(name))
    for key, value in pairs(definition.Stats) do
        monster[key] = value
    end
end

function MonsterManager.Init()
    MonsterManager.Spawned = {}
    for _, wave in ipairs(WaveList) do
        assert(wave.duration > 0,"Wave duration must be positive")
        for _, entry in ipairs(wave.monsters) do
            assert(MonsterList[entry.name],"Unknown wave monster: " .. tostring(entry.name))
            assert(entry.count >= 0 and entry.count == math.floor(entry.count),"Monster count must be a non-negative integer")
        end
    end
end

function MonsterManager.Create(MonsterScriptName,Distance)
    Debug.Log(MonsterScriptName)
    if PlayerManager.transform ~= nil then
        local targetID = Pool.Active(MonsterScriptName)
        Group.Set("MonsterList", targetID)
        local angle = math.random() * math.pi * 2
        local spawnPoint = PlayerManager.transform.position + PVector3(math.cos(angle), math.sin(angle), 0) * Distance

        local rigidbody = Rigidbody2D.Get(targetID)
        rigidbody:SetPosition(spawnPoint)
        Entity.CallFunction(targetID,"ActiveSetting")
    end
end

function MonsterManager.Update(DTime)
    if Time.IsPaused() then return end
    local wave = WaveList[GameTimeManager.CurrentWave]
    while wave do
        local progress = math.min(GameTimeManager.WaveTime / wave.duration,1)
        for index, monsterData in ipairs(wave.monsters) do
            local expectedCount = math.floor(progress * monsterData.count)
            local spawned = MonsterManager.Spawned[index] or 0
            while spawned < expectedCount do
                MonsterManager.Create(monsterData.name,20)
                spawned = spawned + 1
            end
            MonsterManager.Spawned[index] = spawned
        end
        if GameTimeManager.WaveTime < wave.duration then return end
        MonsterManager.NextWave()
        wave = WaveList[GameTimeManager.CurrentWave]
    end
end

function MonsterManager.NextWave()
    local previousWave = WaveList[GameTimeManager.CurrentWave]
    if not previousWave then return end
    GameTimeManager.WaveTime = GameTimeManager.WaveTime - previousWave.duration
    GameTimeManager.CurrentWave = GameTimeManager.CurrentWave + 1
    MonsterManager.Spawned = {}
end
