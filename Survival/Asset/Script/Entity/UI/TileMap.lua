TileMap = {}

TileMap.line = 1
TileMap.corner = 2

TileMap.width = 14
TileMap.height = 14
TileMap.Parent = -1
TileMap.grid = {}

--아무것도 들어있지 않은 14*14 배열을 생성
function TileMap.Create(parentTransform)
    for rowIndex = 0, TileMap.height-1 do
        TileMap.grid[rowIndex] = {}
        for columnIndex = 0, TileMap.width-1 do
            TileMap.grid[rowIndex][columnIndex] = {Value = 0,ID = -1,Rot = 0}
            local targetID = TileMap.CreateTile(PVector3(columnIndex * 48, rowIndex*48, 0))
            parentTransform:SetChild(targetID)
        end
    end
end


function TileMap.Change(x,y,value)
    if y > TileMap.height-1 then
        Debug.LogWarning("Y값이 14를 넘어감"..tostring(y))
        return
    end
    if x > TileMap.width-1 then
        Debug.LogWarning("X값이 14를 넘어감"..tostring(x))
        return
    end
    
    --기존 객체를 지운다
    if TileMap.grid[y][x].Value ~= 0 then
        InventoryFunction.Add(TileMap.grid[y][x].Value,1)
        Entity.Destroy(TileMap.grid[y][x].ID)
    end
    --새로운 객체로 변경
    local targetID = TileMap.CreateObject(PVector3(x * 48,y*48,0),value)
    TileMap.grid[y][x].Value = value
    TileMap.grid[y][x].ID = targetID
    TileMap.grid[y][x].R = 0
    TileMap.Parent:SetChild(targetID) 
end

function TileMap.Delete(x,y)
    if y > TileMap.height-1 then
        Debug.LogWarning("Y값이 14를 넘어감"..tostring(y))
        return
    end

    if x > TileMap.width-1 then
        Debug.LogWarning("X값이 14를 넘어감"..tostring(x))
        return
    end
    --기존 객체를 지운다
    if TileMap.grid[y][x].Value ~= 0 then
        Entity.Destroy(TileMap.grid[y][x].ID)
        TileMap.grid[y][x].Value = 0
    end
end



function TileMap.Rotation(x,y)
    if y > TileMap.height-1 then
        Debug.LogWarning("Y값이 14를 넘어감"..tostring(y))
        return
    end

    if x > TileMap.width-1 then
        Debug.LogWarning("X값이 14를 넘어감"..tostring(x))
        return
    end
    
    if TileMap.grid[y][x].Value == TileMap.line or 
       TileMap.grid[y][x].Value == TileMap.corner then
       
        local tr =  Transform.Get(TileMap.grid[y][x].ID)
        TileMap.grid[y][x].Rot = TileMap.grid[y][x].Rot + 90
        
        if TileMap.grid[y][x].Rot >= 360 then
            TileMap.grid[y][x].Rot = 0
        end
        tr.rotation.z = TileMap.grid[y][x].Rot
    end
end


function TileMap.CreateTile(pos)
    local targetID = Entity.Create("Tile")
    local transform = Transform.Get(targetID)
    transform.position = pos
    return targetID
end

function TileMap.CreateObject(pos,value)
    local targetID = Entity.CreateEmpty()
    local transform = Transform.Add(targetID)
    local image = UIImage.Add(targetID)
    image:SetTextureSize(64,64)
    transform.position = pos
    transform.scale = PVector3(0.75,0.75,1)
    image:SetTexture(ItemList[value].Texture) 
    return targetID
end

