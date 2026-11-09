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

function LuaManager:Remove(ID) 
	self.entities[ID] = nil
end

---등록된 엔티티의 업데이트를 순서대로 실행한다.
function LuaManager:Update(DTime) 
	
	if not self.entities then return end
	isUpdating = true

	--루아의 기본 함수 업데이트 호출
	for id, entityData in pairs(self.entities) do
		if entityData.Update then
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
	for ID in pairs(pendingDestroy) do
		pendingDestroy[ID] = nil
		self:Remove(ID)
		DestroyEntity(ID)
	end
end

---업데이트 중에는 삭제를 예약하고, 그 밖에는 즉시 삭제한다.
function Entity.Destroy(ID)
	if isUpdating or pendingDestroy[ID] then
		pendingDestroy[ID] = true
		return
	end
	LuaManager:Remove(ID)
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
