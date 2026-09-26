-- Default Lua Script
ChoiceManager = {}

ChoiceManager.contentID = 0
ChoiceManager.bookID = 0

function ChoiceManager.Init()
    ChoiceManager.bookID = Entity.Create("ContentBook")
    
    ChoiceManager.contentID = Entity.Create("UpgradeListEntity")
    ChoiceManager.tilemapID = Entity.Create("GridEntity")
    
    Entity.SetActive(ChoiceManager.bookID, false)
end

--책을 여는 애니메이션
function ChoiceManager.BookOpen()
    Entity.SetActive(ChoiceManager.bookID, true)
    Entity.CallFunction(ChoiceManager.bookID,"Open")
end

--책을 다음으로 넘기는 애니메이션
function ChoiceManager.BookNext()
    Entity.SetActive(ChoiceManager.bookID, true)
    Entity.CallFunction(ChoiceManager.bookID,"Next")
end

--책을 덮는 애니메이션
function ChoiceManager.BookClose()
    Entity.SetActive(ChoiceManager.bookID, true)
    Entity.CallFunction(ChoiceManager.bookID,"Close")
end


--책을 열고 content를 보여준다
function ChoiceManager.ContentOpen()
    Entity.SetActive(ChoiceManager.contentID, true)
    Entity.CallFunction(ChoiceManager.contentID,"Setting")
    Entity.CallFunction(ChoiceManager.contentID,"Select",1)
end

--책을 닫고  content를 감춘다
function ChoiceManager.ContentClose()
    Entity.SetActive(ChoiceManager.contentID, false)
end

function ChoiceManager.TilemapOpen()
    Entity.SetActive(ChoiceManager.tilemapID, true)
    Entity.CallFunction(ChoiceManager.tilemapID,"Setting")
end





