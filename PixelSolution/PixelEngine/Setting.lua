local LuaManager = {} 

LuaManager.entities = {}

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

function LuaManager:Update(DTime) 
	
	if not self.entities then return end

	--루아의 기본 함수 업데이트 호출
	for id, entityData in pairs(self.entities) do
		if entityData.Update then
		    entityData.EntityTable:Update(DTime)
		end
	end
end

return LuaManager
