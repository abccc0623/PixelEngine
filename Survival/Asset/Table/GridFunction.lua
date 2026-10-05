---@class GridFunction
GridFunction = {}

local function Cell(x, y)
    return Grid[x] and Grid[x][y]
end

local function Draw(x, y)
    local cell = Cell(x, y)
    local image = Grid.UI and Grid.UI[x] and Grid.UI[x][y]
    local transform = Grid.Transform and Grid.Transform[x] and Grid.Transform[x][y]
    if image then
        image:SetTexture(cell.ItemIndex == 0 and "tile" or ItemList[cell.ItemIndex].Texture)
        image:SetContentAspectFit(18,18)
        image:SetColor(255,255,255,255)
        Entity.SetActive(image.thisID,Grid.Visible == true and cell.ItemIndex ~= 0)
    end
    if transform then transform.rotation.z = cell.ItemRotation end
end

function GridFunction.Add(x, y, itemIndex)
    local cell = Cell(x, y)
    if not cell or not ItemList[itemIndex] then return false end
    if cell.ItemIndex ~= 0 and ItemList[cell.ItemIndex].ItemType == ItemType.Factory and not InventoryFunction.Add(cell.ItemIndex, 1) then
        return false
    end
    cell.ItemIndex = itemIndex
    if itemIndex == 2 then
        Grid.StartPoint = {x = x,y = y}
        Debug.Log(tostring(Grid.StartPoint.y))
    elseif itemIndex == 3 then 
        Grid.EndPoint = {x = x,y = y}
         Debug.Log(tostring(Grid.EndPoint.y))
    end 
    
    cell.ItemRotation = 0
    Draw(x, y)
    Graph.Reset()
    return true
end

function GridFunction.Remove(x, y)
    local cell = Cell(x, y)
    if not cell or cell.ItemIndex == 0 then return false end
    if ItemList[cell.ItemIndex].ItemType == ItemType.Factory and not InventoryFunction.Add(cell.ItemIndex, 1) then return false end
    cell.ItemIndex = 0
    cell.ItemRotation = 0
    Draw(x, y)
    return true
end

function GridFunction.Rotation(x, y)
    local cell = Cell(x, y)
    if not cell then return end
    local item = ItemList[cell.ItemIndex]
    if not item or (item.Texture ~= "line" and item.Texture ~= "corner") then return end
    cell.ItemRotation = (cell.ItemRotation + 90) % 360
    local transform = Grid.Transform and Grid.Transform[x] and Grid.Transform[x][y]
    if transform then transform.rotation.z = cell.ItemRotation end
end

function GridFunction.Get(x, y)
    local cell = Cell(x, y)
    return cell and cell.ItemIndex or 0
end
