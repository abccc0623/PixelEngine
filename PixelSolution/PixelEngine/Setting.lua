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
			if Entity.GetActive(id) then
				entityData.EntityTable:Update(DTime)
			end
		end
	end
end

Entity = Entity or {}

local DestroyEntity = Entity.Destroy

function Entity.Destroy(ID)
	LuaManager:Remove(ID)
	return DestroyEntity(ID)
end

---@return number? ID
function Entity.CreateCamera()
	local id = Entity.CreateEmpty()
	if id == 4294967295 then
		return nil
	end
	Transform.Add(id)
	Camera.Add(id)
	return id
end

---@param textureName string
---@return UIImageData?
function Entity.Create2DImage(textureName)
	if type(textureName) ~= "string" then
		error("Entity.Create2DImage: textureName must be a string",2)
	end
	local id = Entity.CreateEmpty()
	if id == 4294967295 then
		return nil
	end
	Transform.Add(id)
	local image = UIImage.Add(id)
	image:SetTexture(textureName)
	return image
end

function Entity.GetScript(ID)
	local entity = LuaManager.entities[ID]
	if entity == nil then
		return nil
	end

	return entity.EntityTable
end

return LuaManager
