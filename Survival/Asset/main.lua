--- 리소스와 공통 데이터를 불러오고 InGame에서 플레이어 조작을 시작한다.
function Main()
	Asset.Import("./Asset/Font")
	Asset.Import("./Asset/Image")
	Asset.Import("./Asset/Table")
	Asset.Import("./Asset/Script")
	Asset.Import("./Asset/Scene")

	Engine.SetWindowSize(1920, 1080)
	Scene.Change("InGame")
end
