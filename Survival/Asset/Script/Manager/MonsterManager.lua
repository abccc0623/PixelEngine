MonsterManager = {}

GameTimeManager.Waves =
{
    -- 1웨이브
    {
        duration = 5 * 60,
        monsters =
        {
            {
                name = "Bat",
                count = 300/3,
                spawned = 0,
            },
            {
                name = "Skeleton",
                count = 300/7,
                spawned = 0,
            },
        }
    },
    -- 2웨이브
    {
        duration = 5 * 60,
        monsters =
        {
            {
                name = "Skeleton",
                count = 300,
                spawned = 0,
            },
            --[[
            {
                name = "FastMonster",
                count = 60,
                spawned = 0,
            },

            {
                name = "TankMonster",
                count = 20,
                spawned = 0,
            }
            ]]--
        }
    }
}



function MonsterManager.Init()
    --Pool.SetExpansionSize("Bat", 100)
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
    local wave = GameTimeManager.Waves[GameTimeManager.CurrentWave]

    -- 모든 웨이브 종료
    if wave == nil then
        return
    end

    local progress = math.min(GameTimeManager.WaveTime / wave.duration, 1)

    -- 몬스터 종류별 생성
    for _, monsterData in ipairs(wave.monsters) do
        local expectedCount = math.floor(progress * monsterData.count)

        while monsterData.spawned < expectedCount do
            MonsterManager.Create(monsterData.name,20)
            monsterData.spawned = monsterData.spawned + 1
        end
    end

    -- 웨이브 종료
    if GameTimeManager.WaveTime >= wave.duration then
        GameTimeManager.NextWave()
    end
end

function MonsterManager.NextWave()
    local previousWave = GameTimeManager.Waves[GameTimeManager.CurrentWave]

    GameTimeManager.WaveTime = GameTimeManager.WaveTime - previousWave.duration
    GameTimeManager.CurrentWave = GameTimeManager.CurrentWave + 1

    local nextWave = GameTimeManager.Waves[GameTimeManager.CurrentWave]

    GameTimeManager.ResetMonsterSpawnCounts()
end