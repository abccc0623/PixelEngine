---@class UpgradeListFunction
UpgradeListFunction = {}

function UpgradeListFunction.Shuffle()
    local indices = {}

    for i = 1, #ItemList do
        if ItemList[i].Activeupgrade == true then
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
