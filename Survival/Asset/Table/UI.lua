---@class UI
UI = {BookID = -1}

function UI.CreateBookTitleIcon(textureName)
    local id = Entity.CreateEmpty()
    local TR = Transform.Add(id)
    local IM = UIImage.Add(id)
    
    TR.position = PVector3(-204,-145,0)
    TR.scale = PVector3(1,1,1)
    TR:SetParent(UI.BookID)
    IM:SetTexture(textureName)
    IM:SetPivot(0.5,0.5)
    return id
end

function UI.CreateBookTitleText(content)
    local backID = UI.CreateBookTitleBackGround()
    local id = Entity.CreateEmpty()
    local TR = Transform.Add(id)
    local TE = UIText.Add(id)
    TR.position = PVector3(-116.5,-158,0)
    TR.scale = PVector3(0.5,0.5,1)
    TR:SetParent(UI.BookID)
    TR:SetChild(backID)
    TE:SetFont("Jalnan2TTF")
    TE:SetPivot(0.5, 0.5)
    TE:SetText(content)
    TE:SetColor(20,32,50,255)
    return id, backID
end


function UI.CreateBookSubText(x,y,content,lineGap)
    local id = Entity.CreateEmpty()
    local TR = Transform.Add(id)
    local TE = UIText.Add(id)
    TR.position = PVector3(x,y,0)
    TR.scale = PVector3(0.3,0.3,1)
    TR:SetParent(UI.BookID)
    TE:SetFont("Jalnan2TTF")
    TE:SetPivot(0.5, 0.5)
    TE:SetText(content)
    TE:SetColor(20,32,50,255)
    local line1 = UI.CreateBookLine2(lineGap,-5)
    local line2 = UI.CreateBookLine2(-lineGap,-5)
    TR:SetChild(line1)
    TR:SetChild(line2)
    return id, line1, line2
end


function UI.CreateBookLine1(x,y)
    local id = Entity.CreateEmpty()
    local TR = Transform.Add(id)
    local IM = UIImage.Add(id)
    TR.position = PVector3(x,y,0)
    TR.scale = PVector3(2,2,1)
    TR:SetParent(UI.BookID)
    IM:SetTexture("line_1")
    IM:SetPivot(0.5,0.5)
    return id
end

function UI.CreateBookLine2(x,y)
    local id = Entity.CreateEmpty()
    local TR = Transform.Add(id)
    local IM = UIImage.Add(id)
    TR.position = PVector3(x,y,0)
    TR.scale = PVector3(3,3,1)
    IM:SetTexture("line_2")
    IM:SetPivot(0.5,0.5)
    return id
end


function UI.CreateBookTitleBackGround()
     local id = Entity.CreateEmpty()
    local TR = Transform.Add(id)
    local IM = UIImage.Add(id)
    
    TR.position = PVector3(0,0,0)
    TR.scale = PVector3(10,1,1)
    TR:SetParent(UI.BookID)
    IM:SetTexture("TitleBackGround")
    IM:SetPivot(0.5,0.5)
    return id
end


function UI.CreateChoiceImage(x,y)
    local id = Entity.CreateEmpty()
    local TR = Transform.Add(id)
    local IM = UIImage.Add(id)
    
    TR.position = PVector3(x,y,0)
    TR.scale = PVector3(1,1,1)
    TR:SetParent(UI.BookID)
    IM:SetTexture("Choice")
    IM:SetPivot(0.5,0.5)
    local nameID = Entity.CreateEmpty()
    local nameTransform = Transform.Add(nameID)
    nameTransform:SetParent(id)
    nameTransform.position = PVector3(-46,-15.5,0)
    local nameText = UIText.Add(nameID)
    nameText:SetFont("Jalnan2TTF")
    nameText:SetTextSize(0.25)
    nameText:SetPivot(0,0.5)
    nameText:SetColor(37,35,35,255)
    local typeID = Entity.CreateEmpty()
    local typeTransform = Transform.Add(typeID)
    typeTransform:SetParent(id)
    typeTransform.position = PVector3(-46,0,0)
    local typeText = UIText.Add(typeID)
    typeText:SetFont("Jalnan2TTF")
    typeText:SetTextSize(0.2)
    typeText:SetPivot(0,0.5)
    typeText:SetColor(37,35,35,255)
    return id, nameText, typeText, nameID, typeID
end




