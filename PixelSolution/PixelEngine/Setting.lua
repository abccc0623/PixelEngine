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
			if entityData.EntityTable.Active == true then
				entityData.EntityTable:Update(DTime)
			end
		end
	end
end

Entity = Entity or {}

function Entity.GetScript(ID)
	local entity = LuaManager.entities[ID]
	if entity == nil then
		return nil
	end

	return entity.EntityTable
end

function Entity.SetValue(ID, variableName, value)
	local scriptInstance = Entity.GetScript(ID)
	if scriptInstance == nil then
		return false
	end

	scriptInstance[variableName] = value
	return true
end

function Entity.GetValue(ID, variableName)
	local scriptInstance = Entity.GetScript(ID)
	if scriptInstance == nil then
		return nil
	end

	return scriptInstance[variableName]
end

function Entity.CallFunction(ID, functionName, ...)
	local scriptInstance = Entity.GetScript(ID)
	if scriptInstance == nil then
		return false, "Entity has no script"
	end

	local targetFunction = scriptInstance[functionName]
	if type(targetFunction) ~= "function" then
		return false, "Function does not exist: " .. tostring(functionName)
	end

	return pcall(targetFunction, scriptInstance, ...)
end
return LuaManager
