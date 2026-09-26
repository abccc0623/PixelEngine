---@class GridFunction
GridFunction = {}

-- Clockwise in screen coordinates: up, right, down, left.
local directions = {
    {x = 0, y = -1, opposite = 3},
    {x = 1, y = 0, opposite = 4},
    {x = 0, y = 1, opposite = 1},
    {x = -1, y = 0, opposite = 2},
}

local function Cell(x, y)
    return Grid[x] and Grid[x][y]
end

local function IsConveyor(cell)
    local item = cell and ItemList[cell.ItemIndex]
    return item and item.IsConveyor == true
end

local function ConnectionCount(cell)
    local count = 0
    for _, connected in pairs(cell.Connections or {}) do
        if connected then count = count + 1 end
    end
    return count
end

local function Draw(x, y)
    local cell = Cell(x, y)
    local image = GridUI[x] and GridUI[x][y]
    local transform = GridTransform[x] and GridTransform[x][y]
    local texture = cell.ItemIndex == 0 and "tile" or ItemList[cell.ItemIndex].Texture
    if IsConveyor(cell) then
        local links = cell.Connections or {}
        texture = "line"
        -- The source corner image opens right and down at rotation zero.
        if links[2] and links[3] then texture, cell.ItemRotation = "corner", 0
        elseif links[3] and links[4] then texture, cell.ItemRotation = "corner", 90
        elseif links[4] and links[1] then texture, cell.ItemRotation = "corner", 180
        elseif links[1] and links[2] then texture, cell.ItemRotation = "corner", 270
        elseif links[1] or links[3] then cell.ItemRotation = 90
        elseif links[2] or links[4] then cell.ItemRotation = 0 end
    end
    cell.DisplayTexture = texture
    if image then image:SetTexture(texture) end
    if transform then transform.rotation.z = cell.ItemRotation end
end

-- Preserve established pairs when a third neighbor is placed nearby.
function GridFunction.RefreshConnections()
    for x = 1, #Grid do
        for y = 1, #Grid[x] do
            local cell = Grid[x][y]
            cell.Connections = cell.Connections or {}
            for direction, offset in ipairs(directions) do
                local neighbor = Cell(x + offset.x, y + offset.y)
                if not IsConveyor(cell) or not IsConveyor(neighbor)
                    or not neighbor.Connections or not neighbor.Connections[offset.opposite] then
                    cell.Connections[direction] = nil
                end
            end
        end
    end
    -- Stable tie break: board order, then up/right/down/left.
    for x = 1, #Grid do
        for y = 1, #Grid[x] do
            local cell = Grid[x][y]
            if IsConveyor(cell) then
                for direction, offset in ipairs(directions) do
                    local neighbor = Cell(x + offset.x, y + offset.y)
                    if IsConveyor(neighbor) and not cell.Connections[direction]
                        and ConnectionCount(cell) < 2 and ConnectionCount(neighbor) < 2 then
                        cell.Connections[direction] = true
                        neighbor.Connections[offset.opposite] = true
                    end
                end
            end
        end
    end
    for x = 1, #Grid do
        for y = 1, #Grid[x] do Draw(x, y) end
    end
end

function GridFunction.Add(x, y, ItemIndex)
    local cell = Cell(x, y)
    if not cell or not ItemList[ItemIndex] then return false end
    if cell.ItemIndex ~= 0 then
        if InventoryFunction.Add(cell.ItemIndex, 1) == false then return false end
    end
    -- Keep topology when replacing one conveyor item with another.
    if not IsConveyor(cell) or not ItemList[ItemIndex].IsConveyor then
        cell.Connections = {}
    end
    cell.ItemIndex = ItemIndex
    cell.ItemRotation = 0
    GridFunction.RefreshConnections()
    return true
end

function GridFunction.Remove(x, y)
    local cell = Cell(x, y)
    if not cell or cell.ItemIndex == 0 then return false end
    if InventoryFunction.Add(cell.ItemIndex, 1) == false then return false end
    cell.ItemIndex, cell.ItemRotation, cell.Connections = 0, 0, {}
    GridFunction.RefreshConnections()
    return true
end

function GridFunction.Rotation(x, y)
    local cell = Cell(x, y)
    if not IsConveyor(cell) then return end
    -- Connected roads follow neighbors; only an isolated road rotates manually.
    if ConnectionCount(cell) > 0 then return end
    cell.ItemRotation = (cell.ItemRotation + 90) % 180
    Draw(x, y)
end

function GridFunction.Get(x, y)
    local cell = Cell(x, y)
    return cell and cell.ItemIndex or 0
end
