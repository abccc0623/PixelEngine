---@class InventoryFunction
InventoryFunction = {}

---@param InventoryIndex number
function InventoryFunction.Refresh(InventoryIndex)
    local item = User.Inventory[InventoryIndex]
    local component = User.InventoryComponent[InventoryIndex]
    if item == nil or component == nil then return end

    local image = component.ImageComponent
    local text = component.TextComponent
    if image == nil or text == nil then return end

    if item.ItemIndex > 0 and ItemList[item.ItemIndex] ~= nil then
        image:SetTexture(ItemList[item.ItemIndex].Texture)
        image:SetColor(255,255,255,255)
        text:SetText("X"..tostring(item.ItemCount))
    else
        image:SetColor(255,255,255,0)
        text:SetText("")
    end
end

function InventoryFunction.RefreshAll()
    for index = 1, #User.Inventory do InventoryFunction.Refresh(index) end
end

---@param SelectType number
---@param Index number
---@return boolean
function InventoryFunction.Move(SelectType,Index)
    local componentList = nil
    if SelectType == InventoryType.Tool then
        componentList = User.ToolsComponent
    elseif SelectType == InventoryType.Inventory then
        componentList = User.InventoryComponent
    else
        Debug.LogError("잘못된 인벤토리 선택 타입:"..tostring(SelectType))
        return false
    end

    local slot = componentList[Index]
    if slot == nil or slot.TransformComponent == nil or User.SelectTransform == nil then return false end
    User.Selection.Type = SelectType
    User.Selection.Index = Index
    User.SelectTransform.position.x = slot.TransformComponent.position.x
    User.SelectTransform.position.y = slot.TransformComponent.position.y

    if User.InventoryDescriptionID ~= nil then
        Entity.CallFunction(User.InventoryDescriptionID,"Setting",Index,SelectType)
    end
    return true
end

---@param ItemIndex number
---@param ItemCount number
function InventoryFunction.Add(ItemIndex,ItemCount)
    if ItemIndex <= 0 or ItemCount <= 0 then return false end

    --이미 아이템이 들어있다면 개수추가
    for index, value in ipairs(User.Inventory) do
        if value.ItemIndex == ItemIndex then
            value.ItemCount = value.ItemCount + ItemCount
            InventoryFunction.Refresh(index)
            return true
        end
    end

    --아이템이 없다면 빈곳에 아이템 추가 및 UI 업데이트
    for index, value in ipairs(User.Inventory) do
        if value.ItemIndex  == 0 then
            Debug.LogWarning("아이템 추가:"..tostring(ItemIndex))
            value.ItemIndex = ItemIndex
            value.ItemCount = ItemCount
            InventoryFunction.Refresh(index)
            return true
        end
    end

    Debug.LogWarning("인벤토리가 가득 차 아이템을 추가할 수 없음")
    return false
end

---@param InventoryIndex number
function InventoryFunction.Place(InventoryIndex)
    if InventoryIndex < 1 or InventoryIndex > #User.Inventory then
        Debug.LogError("인벤토리 최대칸"..tostring(#User.Inventory).."을 넘어감")
        return
    end
    local target = User.Inventory[InventoryIndex]
    if (target.ItemCount - 1) > 0 then
        target.ItemCount = target.ItemCount - 1
    else
        target.ItemIndex = 0
        target.ItemCount = 0
    end
    InventoryFunction.Refresh(InventoryIndex)
end
