local LuaManager = {} 

LuaManager.entities = {}

function LuaManager:Add(ID,table) 
	self.entities[ID] = 
	{ 
        id = ID, 
        EntityTable = table,
		Spawn = table.Spawn,
        Update = table.Update,
    }
end

function LuaManager:Remove(ID) 
	self.entities[ID] = nil
end

function LuaManager:Update(DTime) 

	if not self.entities then return end

	for id, entityData in pairs(self.entities) do

		if entityData.Update then
		    entityData.EntityTable:Update(DTime)
		end

	end
end

return LuaManager
