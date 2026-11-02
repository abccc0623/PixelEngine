-- Resource visuals, separate from placeable items in ItemList.
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

function EnergyList.GetColor(element)
	return 255,255,255
end

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
            -- A one-frame animation also replaces the previous pooled object's UV.
            index = animation:Create(item.Texture,item.FramesX or 1,item.FramesY or 1,item.AnimationSpeed or 1)
        end
        if index < 0 then return false end
        cache[itemIndex] = index
        animation:SetLoop(index,true)
    end
    animation:SetUseUnscaledTime(unscaled == true)
    animation:Play(index)
    animation:Reset()
    return true
end
