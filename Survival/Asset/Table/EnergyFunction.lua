EnergyFunction = {Items = {}, Distributors = {}, AttackQueue = {}, QueueTime = 0, AttackInterval = 0.2, Gap = 16, MoveSpeed = 40}

function EnergyFunction.GetDistances(path)
    path = path or Graph.pathNodes
    local distances = {0}
    for i = 2, #path do
        local a = Grid[path[i - 1].x][path[i - 1].y].Position
        local b = Grid[path[i].x][path[i].y].Position
        distances[i] = distances[i - 1] + math.sqrt((b.x - a.x)^2 + (b.y - a.y)^2)
    end
    return distances
end

function EnergyFunction.GetPosition(item,distances)
    local path = item.Path
    local index = math.min(item.NextIndex,#path)
    local node, previous = path[index], path[math.max(1,index - 1)]
    local a, b = Grid[previous.x][previous.y].Position, Grid[node.x][node.y].Position
    local length = distances[index] - distances[math.max(1,index - 1)]
    local ratio = length > 0 and (item.Progress - distances[math.max(1,index - 1)]) / length or 0
    return PVector3(a.x + (b.x - a.x) * ratio,a.y + (b.y - a.y) * ratio,b.z)
end

function EnergyFunction.HasSpace(position)
    for _, item in pairs(EnergyFunction.Items) do
        local other = Transform.Get(item.ID).position
        if math.sqrt((other.x - position.x)^2 + (other.y - position.y)^2) < EnergyFunction.Gap - 0.0001 then return false end
    end
    return true
end

function EnergyFunction.Spawn(nodeIndex,attack,splitCount)
    if GameFlow.State ~= "Combat" or Time.IsPaused() then return false end
    local path = Graph.pathNodes
    if not Graph.IsConnected or not path[nodeIndex] then return false end
    local node = path[nodeIndex]
    local position = Grid[node.x][node.y].Position
    if not EnergyFunction.HasSpace(position) then return false end
    Pool.SetExpansionSize("EnergyItem",1)
    local id = Pool.Active("EnergyItem")
    Transform.Get(id).position = PVector3(position.x,position.y,position.z)
    EnergyFunction.Items[id] = {ID = id, Root = Graph.pathNodes, Path = path, Progress = EnergyFunction.GetDistances(path)[nodeIndex], NextIndex = nodeIndex + 1, Attack = attack or 1, SplitCount = splitCount or 0}
    return true
end

function EnergyFunction.Remove(item)
    EnergyFunction.Items[item.ID] = nil
    Pool.Disable("EnergyItem",item.ID)
end

function EnergyFunction.SetPosition(item,distances)
    Transform.Get(item.ID).position = EnergyFunction.GetPosition(item,distances)
end

function EnergyFunction.QueueAttack(item)
    EnergyFunction.AttackQueue[#EnergyFunction.AttackQueue + 1] = {Attack = item.Attack, SplitCount = item.SplitCount, ResourceIndex = item.ResourceIndex}
end

function EnergyFunction.SaveBeforeEdit()
    local saved = {}
    for _, item in pairs(EnergyFunction.Items) do
        local index = math.max(1,math.min(item.NextIndex - 1,#item.Path))
        local node, nextNode = item.Path[index], item.Path[index + 1]
        local distances = EnergyFunction.GetDistances(item.Path)
        local length = nextNode and distances[index + 1] - distances[index] or 0
        saved[#saved + 1] = {Item = item, X = node.x, Y = node.y, NextX = nextNode and nextNode.x, NextY = nextNode and nextNode.y, Ratio = length > 0 and (item.Progress - distances[index]) / length or 0}
    end
    table.sort(saved,function(a,b)
        return a.Item.ID < b.Item.ID
    end)
    EnergyFunction.Saved = saved
end

function EnergyFunction.RestoreAfterEdit()
    if not EnergyFunction.Saved then return end
    local saved = EnergyFunction.Saved
    EnergyFunction.Saved = nil
    EnergyFunction.Items = {}
    for _, entry in ipairs(saved) do
        local path, index
        if Graph.IsConnected then
            for _, route in ipairs(Graph.routes or {Graph.pathNodes}) do
                for i, node in ipairs(route) do
                    if node.x == entry.X and node.y == entry.Y then
                        if not path then path, index = route, i end
                        local nextNode = route[i + 1]
                        if nextNode and nextNode.x == entry.NextX and nextNode.y == entry.NextY then path, index = route, i break end
                    end
                end
                if path and path[index + 1] and path[index + 1].x == entry.NextX and path[index + 1].y == entry.NextY then break end
            end
        end
        local item = entry.Item
        if path then
            local distances = EnergyFunction.GetDistances(path)
            item.Root, item.Path, item.NextIndex = Graph.pathNodes, path, index + 1
            item.Progress = distances[index] + (distances[index + 1] and entry.Ratio * (distances[index + 1] - distances[index]) or 0)
            local position = EnergyFunction.GetPosition(item,distances)
            if EnergyFunction.HasSpace(position) then
                Transform.Get(item.ID).position = position
                EnergyFunction.Items[item.ID] = item
            else
                EnergyFunction.QueueAttack(item)
                EnergyFunction.Remove(item)
            end
        else
            EnergyFunction.QueueAttack(item)
            EnergyFunction.Remove(item)
        end
    end
    if #EnergyFunction.AttackQueue > 0 then EnergyFunction.QueueTime = EnergyFunction.AttackInterval end
end

-- Project another resource onto this route, so different branches share spacing after a merge.
function EnergyFunction.FrontLimit(item,distances)
    local limit = distances[#distances]
    for _, other in pairs(EnergyFunction.Items) do
        if other.ID ~= item.ID then
            local position = Transform.Get(other.ID).position
            for index = 2, #item.Path do
                local a = Grid[item.Path[index - 1].x][item.Path[index - 1].y].Position
                local b = Grid[item.Path[index].x][item.Path[index].y].Position
                local dx, dy = b.x - a.x, b.y - a.y
                local lengthSquared = dx * dx + dy * dy
                local ratio = lengthSquared > 0 and ((position.x - a.x) * dx + (position.y - a.y) * dy) / lengthSquared or -1
                if ratio >= 0 and ratio <= 1 and math.abs(position.x - a.x - dx * ratio) < 0.001 and math.abs(position.y - a.y - dy * ratio) < 0.001 then
                    local progress = distances[index - 1] + (distances[index] - distances[index - 1]) * ratio
                    if progress > item.Progress + 0.0001 or (math.abs(progress - item.Progress) < 0.0001 and other.ID < item.ID) then limit = math.min(limit,progress - EnergyFunction.Gap) end
                end
            end
        end
    end
    return limit
end

function EnergyFunction.Update(deltaTime)
    if GameFlow.State ~= "Combat" or Time.IsPaused() or deltaTime <= 0 then return end
    if #EnergyFunction.AttackQueue > 0 then
        EnergyFunction.QueueTime = EnergyFunction.QueueTime + deltaTime
        if EnergyFunction.QueueTime >= EnergyFunction.AttackInterval then
            local resource = table.remove(EnergyFunction.AttackQueue,1)
            EnergyFunction.QueueTime = 0
            GameFlow.OnEnergyArrived(resource.Attack)
        end
    end
    local ordered = {}
    for _, item in pairs(EnergyFunction.Items) do
        if not Graph.IsConnected or item.Root ~= Graph.pathNodes then
            EnergyFunction.QueueAttack(item)
            EnergyFunction.Remove(item)
        else
            ordered[#ordered + 1] = item
        end
    end
    table.sort(ordered,function(a,b)
        local remainingA = EnergyFunction.GetDistances(a.Path)[#a.Path] - a.Progress
        local remainingB = EnergyFunction.GetDistances(b.Path)[#b.Path] - b.Progress
        if remainingA == remainingB then return a.ID < b.ID end
        return remainingA < remainingB
    end)
    for _, item in ipairs(ordered) do
        if GameFlow.State ~= "Combat" or Time.IsPaused() then return end
        local remaining = EnergyFunction.MoveSpeed * deltaTime
        while remaining > 0 do
            local distances = EnergyFunction.GetDistances(item.Path)
            if item.NextIndex > #item.Path then
                GameFlow.OnEnergyArrived(item.Attack)
                EnergyFunction.Remove(item)
                break
            end
            local boundary = distances[item.NextIndex]
            local target = math.max(item.Progress,math.min(item.Progress + remaining,boundary,EnergyFunction.FrontLimit(item,distances)))
            local movement = target - item.Progress
            item.Progress = target
            remaining = remaining - movement
            EnergyFunction.SetPosition(item,distances)
            if target >= boundary then
                local index = item.NextIndex
                local node = item.Path[index]
                item.NextIndex = index + 1
                if Grid[node.x][node.y].ItemIndex == 5 and item.NextIndex <= #item.Path then
                    local key = node.x .. ":" .. node.y
                    local options = Graph.GetBranches(item.Path,index)
                    local choice = EnergyFunction.Distributors[key] or 1
                    choice = math.min(choice,#options)
                    item.Path = options[choice]
                    EnergyFunction.Distributors[key] = choice % #options + 1
                end
                if item.NextIndex > #item.Path then
                    GameFlow.OnEnergyArrived(item.Attack)
                    EnergyFunction.Remove(item)
                    break
                end
            end
            if movement <= 0 or target < boundary then break end
        end
    end
end
