-- 설치 아이템과 구분되는 자원 표시 데이터다.
EnergyList = {
	{
		Texture = "item_02",
		StartX = 192,
		StartY = 24,
		FrameWidth = 24,
		FrameHeight = 24,
		FrameCount = 8,
		AnimationSpeed = 1,
	},
}

--- 자원의 기본 표시 색상을 반환한다.
function EnergyList.GetColor(element)
	return 255,255,255
end
