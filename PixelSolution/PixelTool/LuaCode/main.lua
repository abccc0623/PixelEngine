--게임 로직에 진입점 입니다 
function Main()
	--스크립트 로드
	Asset.Import("./Asset/BaseCamera.pxm")

	--카메라 객체 생성
	Engine.CreateEntity("BaseCamera")

	--BackGround 변경
	Engine.BackgroundColor(140, 170, 145)
end