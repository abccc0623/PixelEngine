--게임 로직에 진입점 입니다 
function Main()
    Asset.Import("./Asset/Scene")
    Asset.Import("./Asset/Font")
    Asset.Import("./Asset/Image")
    Asset.Import("./Asset/Script")
    Asset.Import("./Asset/Table")
    
    Engine.SetWindowSize(1920, 1080)
    Scene.Change("GridTest")
end
