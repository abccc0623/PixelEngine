-- 전투와 Book UI 사이의 상태 전환 및 시간 제어를 담당한다.
GameFlow = {State = "Initializing"}

function GameFlow.Init()
    GameFlow.State = "Initializing"
    GameFlow.PendingChoices = 0
    Time.SetPaused(false)
end

function GameFlow.StartCombat()
    GameFlow.State = "Combat"
	Event.Call("BuildPlay")
    Event.Call("ShowGridPlay")
    Time.SetPaused(false)
    PlayerManager.CheckLevelUp()
end

function GameFlow.BeginUpgrade(count)
    count = count or 1
    if GameFlow.State == "Choice" then
        GameFlow.PendingChoices = GameFlow.PendingChoices + count
        return
    end
    if GameFlow.State ~= "Combat" then return end
    GameFlow.PendingChoices = count
    GameFlow.State = "Choice"
    Time.SetPaused(true)
	Event.Call("BuildEdit")
    Event.Call("ResetInventorySelection")
    Event.Call("BookOpen")
end

function GameFlow.Choose(index)
    if GameFlow.State ~= "Choice" or not UpgradeListFunction.Choice(index) then return false end
    GameFlow.PendingChoices = GameFlow.PendingChoices - 1
    if GameFlow.PendingChoices > 0 then
        Event.Call("ShowRandomCard")
    else
        GameFlow.State = "Build"
        Event.Call("BookNext")
    end
    return true
end

function GameFlow.CompleteBuild()
    if GameFlow.State ~= "Build" then return end
	Event.Call("StopBuildTest")
    GameFlow.State = "Closing"
    Event.Call("BookClose")
end

function GameFlow.TestBuild()
	if GameFlow.State ~= "Build" then
		return false
	end
	Event.Call("ToggleBuildTest")
	return true
end

function GameFlow.OnBookClosed()
    if GameFlow.State ~= "Closing" then return end
    GameFlow.StartCombat()
end
