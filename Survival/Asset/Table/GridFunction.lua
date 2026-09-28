---@class GridFunction
GridFunction = {}

local function Cell(x, y)
    return Grid[x] and Grid[x][y]
end

local function Draw(x, y)
    local cell = Cell(x, y)
    local image = GridUI[x] and GridUI[x][y]
    local transform = GridTransform[x] and GridTransform[x][y]
    if image then
        image:SetTexture(cell.ItemIndex == 0 and "tile" or ItemList[cell.ItemIndex].Texture)
    end
    if transform then transform.rotation.z = cell.ItemRotation end
end

function GridFunction.Add(x, y, itemIndex)
    local cell = Cell(x, y)
    if not cell or not ItemList[itemIndex] then return false end
    if cell.ItemIndex ~= 0 and not InventoryFunction.Add(cell.ItemIndex, 1) then
        return false
    end
    cell.ItemIndex = itemIndex
    cell.ItemRotation = 0
    Draw(x, y)
    return true
end

function GridFunction.Remove(x, y)
    local cell = Cell(x, y)
    if not cell or cell.ItemIndex == 0 then return false end
    if not InventoryFunction.Add(cell.ItemIndex, 1) then return false end
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
    local transform = GridTransform[x] and GridTransform[x][y]
    if transform then transform.rotation.z = cell.ItemRotation end
end

function GridFunction.Get(x, y)
    local cell = Cell(x, y)
    return cell and cell.ItemIndex or 0
end
