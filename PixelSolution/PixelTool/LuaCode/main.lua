--게임 로직에 진입점 입니다 
function Main()
	--카메라 객체 생성
	local cameraID = Entity.CreateCamera()
	local transform = Transform.Get(cameraID)
	transform.position.z = -20

	--BackGround 변경
	Engine.BackgroundColor(140, 170, 145)
end
