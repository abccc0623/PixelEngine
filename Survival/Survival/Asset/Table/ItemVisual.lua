-- FramesX/FramesY are optional sprite-sheet column/row counts.
ItemVisual = {Icons = {}}

function ItemVisual.Apply(image,item,width,height,unscaled,texture)
    local id = image.thisID
    local state = ItemVisual.Icons[id]
    local x, y = item and item.FramesX, item and item.FramesY
    local frameWidth, frameHeight, frameCount = item and item.FrameWidth, item and item.FrameHeight, item and item.FrameCount
    local region = type(frameWidth) == "number" and frameWidth > 0 and type(frameHeight) == "number" and frameHeight > 0 and type(frameCount) == "number" and frameCount >= 1
    texture = texture or (item and item.Texture)
    local sheet = type(x) == "number" and type(y) == "number" and x >= 1 and y >= 1 and x % 1 == 0 and y % 1 == 0
    local animated = type(texture) == "string" and texture ~= "" and (region or sheet) and texture == item.Texture
    if not animated then
        if state then
            UIImage.Get(state.ID):SetColor(255,255,255,0)
            Entity.SetActive(state.ID,false)
        end
        image:SetColor(255,255,255,texture and 255 or 0)
        if texture then
            image:SetTexture(texture)
            image:SetContentAspectFit(width,height)
        end
        return
    end
    if not state then
        local iconID = Entity.CreateEmpty()
        local transform = Transform.Add(iconID)
        transform:SetParent(id)
        transform.position = PVector3(0,0,0)
        UIImage.Add(iconID)
        Animation2D.Add(iconID)
        state = {ID = iconID, Animations = {}}
        ItemVisual.Icons[id] = state
    end
    local icon = UIImage.Get(state.ID)
    local animation = Animation2D.Get(state.ID)
    local speed = item.AnimationSpeed or 1
    if type(speed) ~= "number" or speed <= 0 then speed = 1 end
    local key
    if region then
        key = texture .. ":region:" .. (item.StartX or 0) .. ":" .. (item.StartY or 0) .. ":" .. frameWidth .. ":" .. frameHeight .. ":" .. frameCount .. ":" .. speed
    else
        key = texture .. ":" .. x .. ":" .. y .. ":" .. speed
    end
    if state.Animations[key] == nil then
        local index
        if region then
            index = animation:CreateRegion(texture,item.StartX or 0,item.StartY or 0,frameWidth,frameHeight,frameCount,speed)
        else
            index = animation:Create(texture,x,y,speed)
        end
        if index < 0 then
            icon:SetColor(255,255,255,0)
            Entity.SetActive(state.ID,false)
            image:SetColor(255,255,255,0)
            return
        end
        state.Animations[key] = index
        animation:SetLoop(state.Animations[key],true)
    end
    animation:SetUseUnscaledTime(unscaled == true)
    if state.Current ~= key then
        animation:Play(state.Animations[key])
        animation:Reset()
        state.Current = key
    end
    icon:SetTexture(texture)
    icon:SetPivot(0.5,0.5)
    icon:SetColor(255,255,255,255)
    icon:SetOrder(1)
    if region then
        local fit = math.min(width / frameWidth,height / frameHeight)
        icon:SetTextureSize(frameWidth * fit,frameHeight * fit)
        Transform.Get(state.ID).scale = PVector3(1,1,1)
    else
        -- Fit the sheet, then undo its row/column scale to fit a single frame.
        icon:SetAspectFit(width * x,height * y)
        Transform.Get(state.ID).scale = PVector3(1 / x,1 / y,1)
    end
    Entity.SetActive(state.ID,true)
    image:SetColor(255,255,255,0)
end
