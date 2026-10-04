---@class UpgradeListFunction
UpgradeListFunction = {}

function UpgradeListFunction.Shuffle()
    local indices = {}

    for i = 1, #ItemList do
        if ItemList[i].ItemType == ItemType.Stat or ItemList[i].ItemType == ItemType.Factory then
            indices[#indices + 1] = i
        end
    end

    assert(#indices > 0, "업그레이드 후보가 없습니다")

    for i = #indices, 2, -1 do
        local randomIndex = math.random(1, i)
        indices[i], indices[randomIndex] = indices[randomIndex], indices[i]
    end
    
    for i = 1, 4 do
       UpgradeList[i].ItemIndex = indices[(i - 1) % #indices + 1]
    end
    
    for i = 1, 4 do
        local itemIndex = UpgradeList[i].ItemIndex
        local item = ItemList[itemIndex]
        local ui = UpgradeUIList[i]
        if ui then
            if ui.TitleComponent then ui.TitleComponent:SetText(item.Name) end
            if ui.TypeComponent then ui.TypeComponent:SetText(item.ItemType == ItemType.Stat and "[능력치]" or "[설치]") end
            if ui.ContentComponent then ui.ContentComponent:SetText(item.UpgradeContent) end
            if ui.ImageComponent then ui.ImageComponent:SetTexture(item.Texture) end
        end
    end
end

---@param UpgradeIndex number
function UpgradeListFunction.Choice(UpgradeIndex)
    local index = UpgradeList[UpgradeIndex].ItemIndex
    local item = ItemList[index]
    if not item or item.ItemType == ItemType.System then return false end
    if item.ItemType == ItemType.Stat then
        if item.UpgradeSelectAction then item.UpgradeSelectAction() end
        return true
    end
    return InventoryFunction.Add(index, 5)
end
