Graph = {startNode = {}, visited = {}}
Node = {x = 0, y = 0, nextNodes ={}}

function Graph.Reset()
    Graph.visited = {}
    if Grid.StartPoint.x == 0 or Grid.StartPoint.y == 0 then 
        return
    end
    local x = Grid.StartPoint.x
    local y = Grid.StartPoint.y
    Graph.visited[x] = Graph.visited[x] or {}
    Graph.visited[x][y] = true
    --시작점 노드를 추가
    local startNode = Graph.CreateNode(x,y)
    Graph.startNode = startNode

    Graph.GetNeighbors(x,y,Graph.startNode)
end

function Graph.GetNeighbors(x,y,node)
    local Direction = {}
    Direction[1] ={ X = x - 1 , Y = y}  --left
    Direction[2] ={ X = x, Y = y - 1}--top
    Direction[3] ={ X = x + 1, Y = y}--right
    Direction[4] = {X = x, Y = y + 1}--bottom


    for i = 1, 4 do
        --배열 밖으로 나갔는지 체크
        if Direction[i].X > 0 and Direction[i].X <= Grid.width and Direction[i].Y > 0 and Direction[i].Y <= Grid.height then
            local id = Grid[Direction[i].X][Direction[i].Y].ItemIndex  
            local nextX = Direction[i].X
            local nextY = Direction[i].Y
            local visited = Graph.visited[nextX] and Graph.visited[nextX][nextY]
            if id ~= 0 and not visited then
                -- 추가할 때 기록하여 같은 좌표를 중복 등록하지 않는다.
                Graph.visited[nextX] = Graph.visited[nextX] or {}
                Graph.visited[nextX][nextY] = true
                local next = Graph.CreateNode(Direction[i].X,Direction[i].Y)
                table.insert(node.nextNodes,next)
                Graph.GetNeighbors(nextX,nextY,next)
            end
        end
    end
end

function Graph.CreateNode(x,y)
    local node = {x= 0, y = 0,nextNodes ={}}
    node.x = x
    node.y = y
    node.index = Grid[x][y].ItemIndex
    return node
end
