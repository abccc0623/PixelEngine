--게임 로직에 진입점 입니다 
function Main()
    Asset.Import("./Asset/Scene")
    Asset.Import("./Asset/Font")
    Asset.Import("./Asset/Image")
    Asset.Import("./Asset/Script")
    Asset.Import("./Asset/UIScript")
    Asset.Import("./Asset/Table")
	
    Scene.Change("Title")
    
    --Entity.Create("GameLogic")
end
