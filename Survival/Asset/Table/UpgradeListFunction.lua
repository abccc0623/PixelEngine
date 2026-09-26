---@class UpgradeListFunction
UpgradeListFunction = {}

function UpgradeListFunction.Shuffle()
    local indices = {}

    for i = 1, #ItemList do
        indices[i] = i
    end

    for i = #indices, 2, -1 do
        local randomIndex = math.random(1, i)
        indices[i], indices[randomIndex] = indices[randomIndex], indices[i]
    end
    
    for i = 1, 4 do
       UpgradeList[i].ItemIndex = indices[i]
    end
    
    for i = 1, 4 do
        local itemIndex = UpgradeList[i].ItemIndex
        UpgradeUIList[i].TitleComponent:SetText(ItemList[itemIndex].Name)
        UpgradeUIList[i].ContentComponent:SetText(ItemList[itemIndex].UpgradeContent)
        UpgradeUIList[i].ImageComponent:SetTexture(ItemList[itemIndex].Texture)
    end
end

---@param UpgradeIndex number
function UpgradeListFunction.Choice(UpgradeIndex)
    local index = UpgradeList[UpgradeIndex].ItemIndex
    InventoryFunction.Add(index, 5)
end
