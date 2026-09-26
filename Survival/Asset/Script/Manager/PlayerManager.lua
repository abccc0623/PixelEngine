PlayerManager = {}

PlayerManager.transform = nil
PlayerManager.Exp = 0
PlayerManager.ExpMax = 3
PlayerManager.Level = 1

local EXP_GROWTH_RATE = 1.5

function PlayerManager.Init()
    local PlayerID = Entity.Create("Player")
    PlayerManager.transform = Transform.Get(PlayerID)
    
    local ExpSliderID = Entity.Create("EXPSlider")
    local CameraID = Entity.Create("FollowCamera")
    PlayerManager.ExpSlider = UISilder.Get(ExpSliderID)
    
    --UISlider Setting
    PlayerManager.ExpSlider:SetRange(0, 3)
    PlayerManager.ExpSlider:SetValue(PlayerManager.Exp)
end

function PlayerManager.AddExp(expPoint)
    PlayerManager.Exp = PlayerManager.Exp + expPoint
    while PlayerManager.Exp >= PlayerManager.ExpMax do
        PlayerManager.Exp = PlayerManager.Exp - PlayerManager.ExpMax
        PlayerManager.LevelUp()
    end
    PlayerManager.ExpSlider:SetValue(PlayerManager.Exp)
end

function PlayerManager.LevelUp()
    PlayerManager.Level = PlayerManager.Level + 1
    PlayerManager.ExpMax = math.floor(PlayerManager.ExpMax * EXP_GROWTH_RATE)
    PlayerManager.ExpSlider:SetRange(0, PlayerManager.ExpMax)
    Time.SetPaused(true)
    ChoiceManager.BookOpen()
end
