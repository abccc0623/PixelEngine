--- 자원 애니메이션을 캐시하고 지정된 시간 기준으로 재생한다.
function EnergyList.Play(animation,cache,itemIndex,unscaled)
	local item = EnergyList[itemIndex]
	if not item then
		Debug.LogWarning("[EnergyList] Invalid ItemIndex: " .. tostring(itemIndex))
		return false
	end
	local index = cache[itemIndex]
	if index == nil then
		if item.FrameCount then
			index = animation:CreateRegion(item.Texture,item.StartX or 0,item.StartY or 0,item.FrameWidth,item.FrameHeight,item.FrameCount,item.AnimationSpeed or 1)
		else
			-- 단일 프레임도 풀 객체에 남아 있는 이전 UV를 교체한다.
			index = animation:Create(item.Texture,item.FramesX or 1,item.FramesY or 1,item.AnimationSpeed or 1)
		end
		if index < 0 then
			return false
		end
		cache[itemIndex] = index
		animation:SetLoop(index,true)
	end
	animation:SetUseUnscaledTime(unscaled == true)
	animation:Play(index)
	animation:Reset()
	return true
end
