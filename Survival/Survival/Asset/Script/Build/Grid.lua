Direction = {None = 0,Up = 1,Down = 2,Left = 3,Right = 4}
Grid = {width = 12,height = 12,RoadGroups = {}}

---인접한 Road의 출력과 입력이 맞는 경우에만 묶고, 셀과 Road 스크립트의 소속 그룹을 갱신합니다.
function Grid.RebuildRoadGroups()
	local build = Grid.BoardID and Entity.GetScript(Grid.BoardID)
	if build and build.IsEditing then
		build:StopTest()
	end
	for _, group in ipairs(Grid.RoadGroups) do
		group.Active = false
	end
	Grid.RoadGroups = {}
	for x = 1, Grid.width do
		for y = 1, Grid.height do
			local cell = Grid[x] and Grid[x][y]
			if cell then
				cell.RoadGroup = nil
				if cell.Script then
					cell.Script.RoadGroup = nil
				end
			end
		end
	end
	local function Matches(port,dx,dy)
		return port and port.x == dx and port.y == dy
	end
	local function Connected(current,neighbor,dx,dy)
		if current == nil or neighbor == nil then
			return false
		end
		return (Matches(current.OutPut,dx,dy) and Matches(neighbor.Input,-dx,-dy))
			or (Matches(current.Input,dx,dy) and Matches(neighbor.OutPut,-dx,-dy))
	end
	local offsets = {{0,-1},{0,1},{-1,0},{1,0}}
	local function NextTile(tile,port,opposite,group)
		if not port then
			return nil
		end
		local x,y = tile.x + port.x,tile.y + port.y
		local cell = Grid[x] and Grid[x][y]
		if cell and cell.RoadGroup == group and cell.Script and Matches(cell.Script[opposite],-port.x,-port.y) then
			return {x = x,y = y,ID = cell.ID}
		end
		return nil
	end
	local function Order(group)
		local start = nil
		for _, tile in ipairs(group.Tiles) do
			local road = Grid[tile.x][tile.y].Script
			if not NextTile(tile,road.Input,"OutPut",group) then
				start = tile
				break
			end
		end
		group.IsLoop = not start
		local current = start or group.Tiles[1]
		local ordered,visited = {},{}
		while current and not visited[current.ID] do
			visited[current.ID] = true
			ordered[#ordered + 1] = current
			local road = Grid[current.x][current.y].Script
			current = NextTile(current,road.OutPut,"Input",group)
		end
		group.Tiles = ordered
		RoadTransport.BuildPath(group)
	end
	local function Collect(x,y,group)
		if x < 1 or x > Grid.width or y < 1 or y > Grid.height then
			return
		end
		local cell = Grid[x] and Grid[x][y]
		if cell == nil or cell.ItemIndex ~= 1 or cell.ID == nil or cell.RoadGroup then
			return
		end
		cell.RoadGroup = group
		if cell.Script then
			cell.Script.RoadGroup = group
			cell.Script:UpdateVisual()
		end
		group.Tiles[#group.Tiles + 1] = {x = x,y = y,ID = cell.ID}
		for _, offset in ipairs(offsets) do
			local nextX,nextY = x + offset[1],y + offset[2]
			if nextX >= 1 and nextX <= Grid.width and nextY >= 1 and nextY <= Grid.height then
				local neighbor = Grid[nextX] and Grid[nextX][nextY]
				if neighbor and neighbor.ItemIndex == 1 and Connected(cell.Script,neighbor.Script,offset[1],offset[2]) then
					Collect(nextX,nextY,group)
				end
			end
		end
	end
	for x = 1, Grid.width do
		for y = 1, Grid.height do
			local cell = Grid[x] and Grid[x][y]
			if cell and cell.ItemIndex == 1 and cell.ID ~= nil and cell.RoadGroup == nil then
				---@class RoadGroup
				---@field Tiles table[] 그룹에 속한 Road의 Grid 인덱스 x, y와 객체 ID 목록
				---@field input PVector2|nil 경로 시작점의 로컬 픽셀 좌표. 투입 위치 제한은 없으며 순환 그룹은 nil
				---@field Output PVector2|nil 출구의 로컬 픽셀 좌표. 순환 그룹은 nil
				---@field IsLoop boolean 순환 여부
				---@field Length number 픽셀 단위 경로 길이
				---@field Resources table[] 선두부터 정렬된 자원 목록
				local group = {Tiles = {}}
				Grid.RoadGroups[#Grid.RoadGroups + 1] = group
				Collect(x,y,group)
				Order(group)
			end
		end
	end
	RoadTransport.Rebind()
end
