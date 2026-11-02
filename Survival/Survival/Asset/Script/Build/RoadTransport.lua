-- 길이와 속도는 BuildSystem의 확대 배율을 제외한 로컬 픽셀 기준이다.
RoadTransport = {Speed = 20,Spacing = 6,Size = 4,Resources = {}}

---정렬된 타일로 이동 경로와 픽셀 단위 시작·끝 좌표를 구성합니다. 투입 위치는 제한하지 않습니다.
---@param group RoadGroup 대상 길 그룹
function RoadTransport.BuildPath(group)
	group.Active = true
	group.Resources = {}
	group.Gaps = {}
	group.HeadDistance = 0
	group.TailDistance = 0
	group.CompressIndex = 1
	group.Segments = {}
	group.TileSegments = {}
	group.Length = 0
	group.input = nil
	group.Output = nil
	local width,height = 240 / Grid.width,240 / Grid.height
	local function Segment(ax,ay,bx,by,tile)
		local length = math.sqrt((bx - ax)^2 + (by - ay)^2)
		if length <= 0 then
			return
		end
		local segment = {AX = ax,AY = ay,BX = bx,BY = by,Length = length,Start = group.Length,x = tile.x,y = tile.y}
		group.Segments[#group.Segments + 1] = segment
		local key = tile.x .. ":" .. tile.y
		group.TileSegments[key] = group.TileSegments[key] or {}
		table.insert(group.TileSegments[key],segment)
		group.Length = group.Length + length
	end
	for index, tile in ipairs(group.Tiles) do
		local road = Grid[tile.x][tile.y].Script
		if road and road.Input and road.OutPut then
			local cx,cy = -120 + (tile.x - 0.5) * width,-120 + (tile.y - 0.5) * height
			local ax,ay = cx + road.Input.x * width / 2,cy + road.Input.y * height / 2
			local bx,by = cx + road.OutPut.x * width / 2,cy + road.OutPut.y * height / 2
			if not group.IsLoop then
				-- 자원 중심을 경계 안쪽에 두어 마주 보는 다른 그룹과도 겹치지 않게 한다.
				if index == 1 then
					ax,ay = ax - road.Input.x * RoadTransport.Size / 2,ay - road.Input.y * RoadTransport.Size / 2
				end
				if index == #group.Tiles then
					bx,by = bx - road.OutPut.x * RoadTransport.Size / 2,by - road.OutPut.y * RoadTransport.Size / 2
				end
			end
			Segment(ax,ay,cx,cy,tile)
			Segment(cx,cy,bx,by,tile)
		end
	end
	if not group.IsLoop and #group.Segments > 0 then
		local first,last = group.Segments[1],group.Segments[#group.Segments]
		group.input = PVector2(first.AX,first.AY)
		group.Output = PVector2(last.BX,last.BY)
	end
end

---경로상 이동 거리를 로컬 픽셀 좌표와 현재 칸 인덱스로 변환합니다.
---@param group RoadGroup 대상 길 그룹
---@param distance number 그룹 경로 기준점부터의 거리
---@return number x 로컬 픽셀 X
---@return number y 로컬 픽셀 Y
---@return number tileX 가로 칸 인덱스
---@return number tileY 세로 칸 인덱스
function RoadTransport.GetPosition(group,distance)
	if group.IsLoop then
		distance = distance % group.Length
	else
		distance = math.max(0,math.min(group.Length,distance))
	end
	local low,high = 1,#group.Segments
	while low < high do
		local middle = math.floor((low + high) / 2)
		local segment = group.Segments[middle]
		if segment.Start + segment.Length < distance then
			low = middle + 1
		else
			high = middle
		end
	end
	local segment = group.Segments[low]
	local t = (distance - segment.Start) / segment.Length
	return segment.AX + (segment.BX - segment.AX) * t,segment.AY + (segment.BY - segment.AY) * t,segment.x,segment.y
end

local function SetResourcePosition(group,resource,distance,loopStep)
	local entry = resource.Entry
	if entry then
		local moved = distance - entry.Start
		if group.IsLoop then
			entry.Traveled = (entry.Traveled or 0) + (loopStep or 0)
			moved = entry.Traveled
		end
		if moved < entry.Length then
			local t = math.max(0,moved / entry.Length)
			resource.X = entry.AX + (entry.BX - entry.AX) * t
			resource.Y = entry.AY + (entry.BY - entry.AY) * t
			resource.TileX,resource.TileY = entry.x,entry.y
			resource.Transform.position = PVector3(resource.X,resource.Y,0)
			return
		end
		resource.Entry = nil
	end
	resource.X,resource.Y,resource.TileX,resource.TileY = RoadTransport.GetPosition(group,distance)
	resource.Transform.position = PVector3(resource.X,resource.Y,0)
end

local function Render(group,loopStep)
	local distance = group.HeadDistance
	for i, resource in ipairs(group.Resources) do
		if i > 1 then
			distance = distance - group.Gaps[i - 1]
		end
		SetResourcePosition(group,resource,distance,loopStep)
	end
end

---들어오는 경계부터 자원을 투입합니다. 옆 합류는 중앙까지 별도 진입 구간으로 이동합니다.
---@param x number 가로 Grid 인덱스
---@param y number 세로 Grid 인덱스
---@param resourceIndex? number EnergyList 인덱스. 기본값은 1
---@return boolean success 투입 성공 여부
---@return table|nil resource 생성된 자원 데이터
local function InsertResource(x,y,resourceIndex,resource,fromX,fromY)
	resourceIndex = resourceIndex or 1
	if type(x) ~= "number" or type(y) ~= "number" then
		return false,nil
	end
	if x % 1 ~= 0 or y % 1 ~= 0 or x < 1 or x > Grid.width or y < 1 or y > Grid.height then
		return false,nil
	end
	local cell = Grid[x] and Grid[x][y]
	local group = cell and cell.RoadGroup
	if not cell or cell.ItemIndex ~= 1 or not group or not group.Active or group.Length <= 0 or not EnergyList[resourceIndex] then
		return false,nil
	end
	local segments = group.TileSegments[x .. ":" .. y]
	if not segments or #segments == 0 then
		return false,nil
	end
	local road = cell.Script
	if not road or not road.Input then
		return false,nil
	end
	local sideX,sideY
	if fromX ~= nil or fromY ~= nil then
		if type(fromX) ~= "number" or type(fromY) ~= "number" then
			return false,nil
		end
		sideX,sideY = fromX - x,fromY - y
		if math.abs(sideX) + math.abs(sideY) ~= 1 or sideX % 1 ~= 0 or sideY % 1 ~= 0 then
			return false,nil
		end
	elseif road and road.Input then
		sideX,sideY = road.Input.x,road.Input.y
	else
		return false,nil
	end
	local first = segments[1]
	local center = first.Start + first.Length
	local ax = first.BX + sideX * (240 / Grid.width) / 2
	local ay = first.BY + sideY * (240 / Grid.height) / 2
	local entry = nil
	-- 그룹 간 이동은 기존 좌표에서 이어서 움직이며 객체를 재생성하지 않는다.
	if resource then
		ax,ay = resource.X,resource.Y
	end
	local length = math.sqrt((first.BX - ax)^2 + (first.BY - ay)^2)
	local target = center - length
	if length > 0 then
		entry = {AX = ax,AY = ay,BX = first.BX,BY = first.BY,Length = length,x = x,y = y}
	end
	if group.IsLoop then
		target = target % group.Length
	end
	if entry then
		entry.Start = target
	end
	local entries = {}
	local distance = group.HeadDistance
	for i, resource in ipairs(group.Resources) do
		if i > 1 then
			distance = distance - group.Gaps[i - 1]
		end
		local position = group.IsLoop and (distance % group.Length) or distance
		local separation = math.abs(position - target)
		if group.IsLoop then
			separation = math.min(separation,group.Length - separation)
		end
		if separation < RoadTransport.Spacing - 0.000001 then
			return false,nil
		end
		entries[#entries + 1] = {Resource = resource,Distance = position}
	end
	if group.IsLoop then
		-- 순환 경계가 자원 목록 중간에 있어도 현재 거리 기준으로 순서를 맞춘다.
		table.sort(entries,function(a,b)
			return a.Distance > b.Distance
		end)
	end
	local insertion = 1
	while insertion <= #entries and entries[insertion].Distance > target do
		insertion = insertion + 1
	end
	if not resource then
		local id = Entity.Create("BuildEnergy")
		if not id or id == 4294967295 then
			return false,nil
		end
		local script = Entity.GetScript(id)
		if not script or not script:Setting(resourceIndex) then
			Entity.Destroy(id)
			return false,nil
		end
		resource = {ID = id,Script = script,Transform = script.transform,ResourceIndex = resourceIndex}
		RoadTransport.Resources[#RoadTransport.Resources + 1] = resource
	end
	resource.Group = group
	resource.Entry = entry
	table.insert(entries,insertion,{Resource = resource,Distance = target})
	group.Resources = {}
	group.Gaps = {}
	for i, entry in ipairs(entries) do
		group.Resources[i] = entry.Resource
		if i > 1 then
			group.Gaps[i - 1] = entries[i - 1].Distance - entry.Distance
		end
	end
	group.HeadDistance = entries[1].Distance
	group.TailDistance = entries[#entries].Distance
	group.CompressIndex = 1
	SetResourcePosition(group,resource,target)
	return true,resource
end

---길의 입력 경계에 새 자원을 투입합니다. 출발 칸을 지정하면 그쪽 경계로 들어옵니다.
---@param x number 가로 Grid 인덱스
---@param y number 세로 Grid 인덱스
---@param resourceIndex? number EnergyList 인덱스
---@param fromX? number 자원을 보내는 칸의 가로 인덱스
---@param fromY? number 자원을 보내는 칸의 세로 인덱스
---@return boolean success 투입 성공 여부
---@return table|nil resource 생성된 자원
function RoadTransport.input(x,y,resourceIndex,fromX,fromY)
	return InsertResource(x,y,resourceIndex,nil,fromX,fromY)
end

local function RemoveHead(group)
	local resource = table.remove(group.Resources,1)
	if #group.Resources > 0 then
		group.HeadDistance = group.HeadDistance - table.remove(group.Gaps,1)
	else
		group.HeadDistance = 0
		group.TailDistance = 0
		group.Gaps = {}
	end
	group.CompressIndex = 1
	return resource
end

local function TransferOutput(group)
	if not group.Active or group.IsLoop or #group.Resources == 0 or group.HeadDistance < group.Length - 0.000001 then
		return
	end
	local last = group.Tiles[#group.Tiles]
	local road = last and Grid[last.x] and Grid[last.x][last.y].Script
	if not road or not road.OutPut then
		return
	end
	local direction = road.OutPut
	local x,y = last.x + direction.x,last.y + direction.y
	local cell = Grid[x] and Grid[x][y]
	local target = cell and cell.RoadGroup
	local targetRoad = cell and cell.Script
	if not cell or cell.ItemIndex ~= 1 or not target or target == group or not targetRoad or not targetRoad.OutPut then
		return
	end
	-- 옆 합류는 허용하되 서로 출구가 마주 보는 역방향 길로는 보내지 않는다.
	if targetRoad.OutPut.x == -direction.x and targetRoad.OutPut.y == -direction.y then
		return
	end
	local resource = group.Resources[1]
	if InsertResource(x,y,resource.ResourceIndex,resource,last.x,last.y) then
		RemoveHead(group)
	end
end

---지정한 길의 출구에 도착한 선두 자원을 제거하고 데이터를 반환합니다.
---@param x number 가로 Grid 인덱스
---@param y number 세로 Grid 인덱스
---@return table|nil resource 회수한 자원 데이터
function RoadTransport.TakeOutput(x,y)
	local cell = Grid[x] and Grid[x][y]
	local group = cell and cell.RoadGroup
	if not cell or cell.ItemIndex ~= 1 or not group or not group.Active or group.IsLoop then
		return nil
	end
	local last = group.Tiles[#group.Tiles]
	if not last or last.x ~= x or last.y ~= y or #group.Resources == 0 or group.HeadDistance < group.Length - 0.000001 then
		return nil
	end
	local resource = RemoveHead(group)
	for i, entry in ipairs(RoadTransport.Resources) do
		if entry == resource then
			table.remove(RoadTransport.Resources,i)
			break
		end
	end
	resource.Group = nil
	if Entity.GetScript(resource.ID) == resource.Script then
		Entity.Destroy(resource.ID)
	end
	return resource
end

---그룹의 선두 거리와 자원 간격을 갱신합니다. 자원별 이동 판정은 하지 않습니다.
---@param deltaTime number 게임 시간 기준 델타 초
function RoadTransport.Update(deltaTime)
	local step = RoadTransport.Speed * deltaTime
	if step <= 0 then
		return
	end
	for _, group in ipairs(Grid.RoadGroups) do
		if #group.Resources > 0 and group.Length > 0 then
			if group.IsLoop then
				local span = group.HeadDistance - group.TailDistance
				group.HeadDistance = (group.HeadDistance + step) % group.Length
				group.TailDistance = group.HeadDistance - span
				Render(group,step)
			else
				local headMove = math.min(step,math.max(0,group.Length - group.HeadDistance))
				group.HeadDistance = group.HeadDistance + headMove
				local remaining = step - headMove
				local compressed = 0
				-- 이미 압축된 간격은 건너뛰고 다음 여유 간격만 추적한다.
				while remaining > 0 and group.CompressIndex <= #group.Gaps do
					local index = group.CompressIndex
					local amount = math.min(remaining,math.max(0,group.Gaps[index] - RoadTransport.Spacing))
					group.Gaps[index] = group.Gaps[index] - amount
					compressed = compressed + amount
					remaining = remaining - amount
					if group.Gaps[index] <= RoadTransport.Spacing + 0.000001 then
						group.CompressIndex = index + 1
					else
						break
					end
				end
				group.TailDistance = group.TailDistance + headMove + compressed
				if headMove > 0 or compressed > 0 then
					Render(group)
				end
			end
		end
	end
	-- 모든 그룹의 이동을 끝낸 뒤 넘겨서 그룹 순서에 따라 두 번 이동하지 않게 한다.
	for _, group in ipairs(Grid.RoadGroups) do
		TransferOutput(group)
	end
end

local function Project(resource,segments)
	local bestDistance,bestError = 0,math.huge
	for _, segment in ipairs(segments) do
		local dx,dy = segment.BX - segment.AX,segment.BY - segment.AY
		local t = math.max(0,math.min(1,((resource.X - segment.AX) * dx + (resource.Y - segment.AY) * dy) / segment.Length^2))
		local error = (resource.X - segment.AX - dx * t)^2 + (resource.Y - segment.AY - dy * t)^2
		if error < bestError then
			bestError = error
			bestDistance = segment.Start + t * segment.Length
		end
	end
	return bestDistance
end

---그룹이 바뀌어도 자원을 유지하며 같은 타일의 새 경로에 재배치합니다.
function RoadTransport.Rebind()
	local pending = {}
	for _, resource in ipairs(RoadTransport.Resources) do
		resource.Group = nil
		local entering = resource.Entry
		resource.Entry = nil
		local cell = Grid[resource.TileX] and Grid[resource.TileX][resource.TileY]
		local group = cell and cell.RoadGroup
		local segments = group and group.TileSegments[resource.TileX .. ":" .. resource.TileY]
		if segments and #segments > 0 then
			local distance = Project(resource,segments)
			if entering then
				local first = segments[1]
				local length = math.sqrt((first.BX - resource.X)^2 + (first.BY - resource.Y)^2)
				distance = first.Start + first.Length - length
				if length > 0 then
					resource.Entry = {
						AX = resource.X,AY = resource.Y,BX = first.BX,BY = first.BY,
						Length = length,x = resource.TileX,y = resource.TileY,
					}
				end
			end
			if group.IsLoop then
				distance = distance % group.Length
			end
			if resource.Entry then
				resource.Entry.Start = distance
			end
			pending[group] = pending[group] or {}
			table.insert(pending[group],{Resource = resource,Distance = distance})
		end
	end
	for group, entries in pairs(pending) do
		table.sort(entries,function(a,b)
			if a.Distance == b.Distance then
				return a.Resource.ID < b.Resource.ID
			end
			return a.Distance > b.Distance
		end)
		local spacing = RoadTransport.Spacing
		local capacity = math.floor(group.Length / spacing) + 1
		if group.IsLoop then
			capacity = capacity - 1
		end
		local count = math.min(#entries,capacity)
		local positions = {}
		for i = 1, count do
			local position = entries[i].Distance
			if i > 1 then
				position = math.min(position,positions[i - 1] - spacing)
			end
			positions[i] = position
		end
		if count > 0 then
			local minimum = group.IsLoop and (positions[1] - group.Length + spacing) or math.min(0,entries[count].Distance)
			if positions[count] < minimum then
				positions[count] = minimum
				for i = count - 1, 1, -1 do
					positions[i] = math.max(positions[i],positions[i + 1] + spacing)
				end
			end
			for i = 1, count do
				local resource = entries[i].Resource
				resource.Group = group
				group.Resources[i] = resource
				if i > 1 then
					group.Gaps[i - 1] = positions[i - 1] - positions[i]
				end
			end
			group.HeadDistance = positions[1]
			group.TailDistance = positions[count]
			Render(group)
		end
	end
	-- 길이 없거나 새 그룹에 자리가 모자란 기존 자원은 그 위치에 보존한다.
	-- 다음 길 수정으로 다시 편입될 수 있으며, 새 자원 투입 대기열과는 별개다.
end

---현재 시뮬레이션의 자원만 제거합니다. 길의 연결 상태는 유지합니다.
function RoadTransport.ClearResources()
	for _, resource in ipairs(RoadTransport.Resources) do
		-- 씬 변경으로 ID가 재사용됐다면 다른 객체를 삭제하지 않는다.
		if Entity.GetScript(resource.ID) == resource.Script then
			Entity.Destroy(resource.ID)
		end
	end
	RoadTransport.Resources = {}
	for _, group in ipairs(Grid.RoadGroups) do
		group.Resources = {}
		group.Gaps = {}
		group.HeadDistance = 0
		group.TailDistance = 0
		group.CompressIndex = 1
	end
end

---새 보드를 만들 때만 기존 자원과 그룹을 초기화합니다.
function RoadTransport.Reset()
	RoadTransport.ClearResources()
	for _, group in ipairs(Grid.RoadGroups) do
		group.Active = false
	end
end
