PlayerManager = {}

PlayerManager.transform = nil
PlayerState = {Speed = 5}
PlayerManager.Exp = 0
PlayerManager.ExpMax = 3
PlayerManager.Level = 1

local EXP_GROWTH_RATE = 1.5

function PlayerManager.Init()
    PlayerManager.Exp = 0
    PlayerManager.ExpMax = 3
    PlayerManager.Level = 1
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
    PlayerManager.CheckLevelUp()
end

function PlayerManager.CheckLevelUp()
    if GameFlow.State == "Combat" or GameFlow.State == "Choice" then
        local count = 0
        while PlayerManager.Exp >= PlayerManager.ExpMax do
            PlayerManager.Exp = PlayerManager.Exp - PlayerManager.ExpMax
            PlayerManager.LevelUp()
            count = count + 1
        end
        if count > 0 then GameFlow.BeginUpgrade(count) end
    end
    PlayerManager.ExpSlider:SetValue(PlayerManager.Exp)
end

function PlayerManager.LevelUp()
    PlayerManager.Level = PlayerManager.Level + 1
    PlayerManager.ExpMax = math.floor(PlayerManager.ExpMax * EXP_GROWTH_RATE)
    PlayerManager.ExpSlider:SetRange(0, PlayerManager.ExpMax)
end

function PlayerManager.Attack(attack,itemIndex)
	if PlayerManager.transform == nil then
		return
	end
	BulletManager.Create(attack,itemIndex)
end
