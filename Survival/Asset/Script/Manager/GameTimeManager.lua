-- Default Lua Script
GameTimeManager = {}

GameTimeManager.Time = 0
GameTimeManager.WaveTime = 0
GameTimeManager.CurrentWave = 1

function GameTimeManager.Init()
    Entity.Create("GameTimeText")

    GameTimeManager.Time = 0
    GameTimeManager.WaveTime = 0
    GameTimeManager.CurrentWave = 1
end

function GameTimeManager.Update(DTime)
    GameTimeManager.Time = GameTimeManager.Time + DTime
    GameTimeManager.WaveTime = GameTimeManager.WaveTime + DTime
    
end


