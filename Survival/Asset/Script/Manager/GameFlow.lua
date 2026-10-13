-- 전투와 Book UI 사이의 상태 전환 및 시간 제어를 담당한다.
GameFlow = {State = "Initializing"}

function GameFlow.Init()
    GameFlow.State = "Initializing"
    Time.SetPaused(false)
end

function GameFlow.StartCombat()
    GameFlow.State = "Combat"
    EnergyFunction.RestoreAfterEdit()
    Event.Call("ShowGridPlay")
    Time.SetPaused(false)
    PlayerManager.CheckLevelUp()
end

function GameFlow.OnEnergyArrived(attack)
    if GameFlow.State ~= "Combat" then return end
    PlayerManager.Attack(attack)
end

function GameFlow.BeginUpgrade()
    if GameFlow.State ~= "Combat" then return end
    GameFlow.State = "Choice"
    Time.SetPaused(true)
    EnergyFunction.SaveBeforeEdit()
    Event.Call("BookOpen")
end

function GameFlow.Choose(index)
    if GameFlow.State ~= "Choice" or not UpgradeListFunction.Choice(index) then return false end
    GameFlow.State = "Build"
    Event.Call("BookNext")
    return true
end

function GameFlow.CompleteBuild()
    if GameFlow.State ~= "Build" then return end
    GameFlow.State = "Closing"
    Event.Call("BookClose")
end

function GameFlow.OnBookClosed()
    if GameFlow.State ~= "Closing" then return end
    GameFlow.StartCombat()
end
