local LuaManager = {} 

LuaManager.entities = {}
local pendingDestroy = {}
local isUpdating = false
Entity = Entity or {}
local DestroyEntity = Entity.Destroy

function LuaManager:Add(ID,scriptInstance) 
	self.entities[ID] = 
	{ 
        id = ID, 
        EntityTable = scriptInstance,
		Spawn = scriptInstance.Spawn,
        Update = scriptInstance.Update,
    }

	--루아의 기본 함수 Spawn 호출
	if self.entities[ID].Spawn then
		self.entities[ID].EntityTable:Spawn()
	end
end

function LuaManager:ResetScene()
    self.entities = {}
    pendingDestroy = {}
    isUpdating = false
end

function LuaManager:Remove(ID) 
	self.entities[ID] = nil
end

---등록된 엔티티의 업데이트를 순서대로 실행한다.
function LuaManager:Update(DTime) 
	
	if not self.entities then return end
	isUpdating = true
	local currentFrame = {}
	for id, entityData in pairs(self.entities) do
		currentFrame[#currentFrame + 1] = {id = id, data = entityData}
	end

	--루아의 기본 함수 업데이트 호출
	for _, entry in ipairs(currentFrame) do
		local id = entry.id
		local entityData = entry.data
		if self.entities[id] == entityData and not pendingDestroy[id] and entityData.Update then
			if Entity.GetActive(id) then
				entityData.EntityTable:Update(DTime)
			end
		end
	end
	isUpdating = false
end

---업데이트 중 요청한 삭제를 실제 엔진에 반영한다.
function LuaManager:FlushPendingDestroy()
	isUpdating = false
	local ids = {}
	for ID in pairs(pendingDestroy) do
		pendingDestroy[ID] = nil
		ids[#ids + 1] = ID
	end
	return ids
end

---생성 중인 객체가 포함된 삭제는 무시하고, 완료 객체만 기존 삭제 규칙을 따른다.
function Entity.Destroy(ID)
	if not LuaManager.CanDestroy(ID) then
		return
	end
	if isUpdating or pendingDestroy[ID] then
		pendingDestroy[ID] = true
		return
	end
	-- 실제 삭제가 승인된 경우에만 네이티브 경로에서 Lua 등록을 제거한다.
	return DestroyEntity(ID)
end

function Entity.GetScript(ID)
	local entity = LuaManager.entities[ID]
	if entity == nil then
		return nil
	end

	return entity.EntityTable
end

return LuaManager
