Graph = {IsConnected = false, startNode = nil, endNode = nil, pathNodes = {}, visited = {}}
Node = {x = 0, y = 0, previousNode = nil, nextNodes = {}}

function Graph.Reset()
    Graph.IsConnected = false
    Graph.ClearRoads()
    Graph.startNode = nil
    Graph.endNode = nil
    Graph.pathNodes = {}
    Graph.visited = {}
    Graph.routes = {}
    Graph.routeKeys = {}
    Graph.branches = {}
    local start = Grid.StartPoint
    local goal = Grid.EndPoint
    local startCell = start and Grid[start.x] and Grid[start.x][start.y]
    local endCell = goal and Grid[goal.x] and Grid[goal.x][goal.y]
    if not startCell or not endCell or startCell.ItemIndex ~= 2 or endCell.ItemIndex ~= 3 then
        Graph.Fail()
        return false
    end

    local endNode, visited = Graph.FindPath(start)
    Graph.visited = visited or {}
    if not endNode then Graph.Fail() return false end
    Graph.CreatePath(endNode)
    Graph.IsConnected = true
    Graph.PrepareRoutes()
    Graph.Success()
    return true
end

function Graph.FindPath(start,blocked)
    local first = Graph.CreateNode(start.x,start.y)
    first.g = 0
    local open = {first}
    local nodes = {[start.x] = {[start.y] = first}}
    local visited = {}
    local goal = Grid.EndPoint
    while #open > 0 do
        local best = 1
        for i = 2, #open do
            if open[i].f < open[best].f or (open[i].f == open[best].f and open[i].h < open[best].h) then best = i end
        end
        local current = table.remove(open,best)
        visited[current.x] = visited[current.x] or {}
        visited[current.x][current.y] = true
        if current.x == goal.x and current.y == goal.y then
            return current, visited
        end

        for _, position in ipairs(Graph.GetNeighbors(current.x,current.y)) do
            local x, y = position.x, position.y
            if not (visited[x] and visited[x][y]) and not (blocked and blocked[x] and blocked[x][y]) then
                nodes[x] = nodes[x] or {}
                local nextNode = nodes[x][y]
                if nextNode == nil then
                    nextNode = Graph.CreateNode(x,y)
                    nodes[x][y] = nextNode
                    open[#open + 1] = nextNode
                end
                local cost = current.g + 1
                if cost < nextNode.g then
                    nextNode.g = cost
                    nextNode.f = cost + nextNode.h
                    nextNode.previousNode = current
                end
            end
        end
    end
    return nil
end

function Graph.RegisterRoute(path)
    local keys = {}
    for i, node in ipairs(path) do keys[i] = node.x .. ":" .. node.y end
    local key = table.concat(keys,",")
    if Graph.routeKeys[key] then return Graph.routeKeys[key] end
    Graph.routeKeys[key] = path
    Graph.routes[#Graph.routes + 1] = path
    return path
end

function Graph.GetBranches(path,index)
    Graph.branches[path] = Graph.branches[path] or {}
    if Graph.branches[path][index] then return Graph.branches[path][index] end
    local options = {path}
    local blocked = {}
    for i = 1, index do
        local node = path[i]
        blocked[node.x] = blocked[node.x] or {}
        blocked[node.x][node.y] = true
    end
    local source, forward = path[index], path[index + 1]
    for _, exit in ipairs(Graph.GetNeighbors(source.x,source.y)) do
        if #options >= 2 then break end
        if not (blocked[exit.x] and blocked[exit.x][exit.y]) and
            not (forward and exit.x == forward.x and exit.y == forward.y) then
            local last = Graph.FindPath(exit,blocked)
            if last then
                local tail = {}
                while last do table.insert(tail,1,last) last = last.previousNode end
                local branch = {}
                for i = 1, index do branch[i] = path[i] end
                for _, node in ipairs(tail) do branch[#branch + 1] = node end
                options[#options + 1] = Graph.RegisterRoute(branch)
            end
        end
    end
    Graph.branches[path][index] = options
    table.sort(options,function(a,b)
        local first, second = a[index + 1], b[index + 1]
        if first.x == second.x then return first.y < second.y end
        return first.x < second.x
    end)
    source.nextNodes = {}
    for i, route in ipairs(options) do source.nextNodes[i] = route[index + 1] end
    return options
end

function Graph.PrepareRoutes()
    Graph.RegisterRoute(Graph.pathNodes)
    local seen, routeIndex = {}, 1
    while routeIndex <= #Graph.routes do
        local path = Graph.routes[routeIndex]
        for index, node in ipairs(path) do
            local key = node.x .. ":" .. node.y
            if node.index == 5 and index < #path and not seen[key] then
                seen[key] = true
                Graph.GetBranches(path,index)
            end
        end
        routeIndex = routeIndex + 1
    end
    -- Both outlets are live roads, not unused road_3 cells.
    for i = #Graph.routes, 1, -1 do
        local path = Graph.routes[i]
        for index, node in ipairs(path) do Graph.ApplyRoad(node,path[index - 1],path[index + 1]) end
    end
end

function Graph.GetNeighbors(x,y)
    local positions = {
        {x = x - 1, y = y},
        {x = x, y = y - 1},
        {x = x + 1, y = y},
        {x = x, y = y + 1},
    }
    local neighbors = {}
    for _, position in ipairs(positions) do
        if position.x >= 1 and position.x <= Grid.width and position.y >= 1 and position.y <= Grid.height then
            local cell = Grid[position.x] and Grid[position.x][position.y]
            if cell and cell.ItemIndex ~= 0 then neighbors[#neighbors + 1] = position end
        end
    end
    return neighbors
end

function Graph.CreateNode(x,y)
    local distance = math.abs(x - Grid.EndPoint.x) + math.abs(y - Grid.EndPoint.y)
    return {x = x, y = y, index = Grid[x][y].ItemIndex, directionIndex = 0, g = math.huge, h = distance, f = distance, previousNode = nil, nextNodes = {}}
end

function Graph.CreatePath(endNode)
    Graph.endNode = endNode
    local node = endNode
    while node ~= nil do
        table.insert(Graph.pathNodes,1,node)
        node = node.previousNode
    end
    Graph.startNode = Graph.pathNodes[1]
    for i = 1, #Graph.pathNodes - 1 do
        Graph.pathNodes[i].nextNodes[1] = Graph.pathNodes[i + 1]
    end
    for i, pathNode in ipairs(Graph.pathNodes) do
        Graph.ApplyRoad(pathNode,Graph.pathNodes[i - 1],Graph.pathNodes[i + 1])
    end
end

function Graph.ClearRoads()
    for x = 1, Grid.width do
        for y = 1, Grid.height do
            local cell = Grid[x] and Grid[x][y]
            if cell and cell.ItemIndex == 1 then
                -- A* 경로의 길만 정상 이미지로 다시 변경한다.
                cell.RoadTexture = "road_3"
                cell.ItemRotation = 0
                Graph.DrawRoad(x,y)
            end
        end
    end
end

function Graph.ApplyRoad(node,previousNode,nextNode)
    if node.index ~= 1 or previousNode == nil or nextNode == nil then return end
    -- 현재 노드에 연결된 방향: 좌 1, 상 2, 우 4, 하 8.
    local mask = 0
    for _, connected in ipairs({previousNode,nextNode}) do
        local dx, dy = connected.x - node.x, connected.y - node.y
        if dx == -1 and dy == 0 then mask = mask + 1
        elseif dx == 0 and dy == -1 then mask = mask + 2
        elseif dx == 1 and dy == 0 then mask = mask + 4
        elseif dx == 0 and dy == 1 then mask = mask + 8 end
    end
    -- 가로, 세로, 우+하, 좌+하, 우+상, 좌+상.
    local indices = {[5] = 1, [10] = 2, [12] = 3, [9] = 4, [6] = 5, [3] = 6}
    node.directionIndex = indices[mask] or 0
    if node.directionIndex == 0 then return end
    local rotations = {0,90,0,90,270,180}
    local cell = Grid[node.x][node.y]
    cell.RoadTexture = node.directionIndex <= 2 and "road_1" or "road_2"
    cell.ItemRotation = rotations[node.directionIndex]
    Graph.DrawRoad(node.x,node.y)
end

function Graph.DrawRoad(x,y)
    local cell = Grid[x][y]
    local image = Grid.UI and Grid.UI[x] and Grid.UI[x][y]
    local transform = Grid.Transform and Grid.Transform[x] and Grid.Transform[x][y]
    if image then
        Graph.SetRoadImage(image,cell.RoadTexture or ItemList[cell.ItemIndex].Texture)
    end
    if transform then transform.rotation.z = cell.ItemRotation end
end

function Graph.SetRoadImage(image,texture)
    image:SetTexture(texture)
    if texture == "road_2" then
        -- 그림 영역 10x10 안의 동그라미 중심은 (2,2).
        -- 직선의 16픽셀을 18로 그리는 것과 같은 배율을 적용한다.
        image:SetContentAspectFit(11.25,11.25)
        image:SetPivot(0.2,0.2)
    else
        image:SetContentAspectFit(18,18)
        image:SetPivot(0.5,0.5)
    end
end

function Graph.Success()
    Graph.SpawnEnergy()
end

function Graph.SpawnEnergy()
    return EnergyFunction.Spawn(1,1)
end

function Graph.Fail()
    Graph.IsConnected = false
    for x = 1, Grid.width do
        for y = 1, Grid.height do
            local cell = Grid[x] and Grid[x][y]
            if cell and cell.ItemIndex == 1 then
                cell.RoadTexture = "road_3"
                cell.ItemRotation = 0
                Graph.DrawRoad(x,y)
            end
        end
    end
end
