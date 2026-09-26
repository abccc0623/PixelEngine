local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

ffi.cdef[[
	typedef struct {
		unsigned int thisID;
	} Rigidbody2DData;

	Rigidbody2DData* Rigidbody2D_Add(unsigned int ID);
	Rigidbody2DData* Rigidbody2D_Get(unsigned int ID);
	bool Rigidbody2D_Has(unsigned int ID);
	void Rigidbody2D_CreateBoxCollider(unsigned int ID,PVector2 Size,PVector2 Center);
	void Rigidbody2D_CreateCircleCollider(unsigned int ID,float Radius,PVector2 Center);
	void Rigidbody2D_SetMotionType(unsigned int ID,MotionType MotionType);
	void Rigidbody2D_SetPosition(unsigned int ID,PVector3 position);
	void Rigidbody2D_SetRotation(unsigned int ID,PVector3 rotation);
	void Rigidbody2D_SetVelocity(unsigned int ID,PVector3 velocity);
	void Rigidbody2D_SetGravity(unsigned int ID,float Gravity);
	void Rigidbody2D_SetRestitution(unsigned int ID,float Restitution);
	void Rigidbody2D_SetFriction(unsigned int ID,float Friction);
	void Rigidbody2D_SetLinearDamping(unsigned int ID,float LinearDamping);
	void Rigidbody2D_SetSensor(unsigned int ID,bool Sensor);
	void Rigidbody2D_SetActive(unsigned int ID,bool Active);
	void Rigidbody2D_SetLayer(unsigned int ID,const char* LayerName);
	const char* Rigidbody2D_GetLayer(unsigned int ID);
]]

---@class Rigidbody2DData
local Rigidbody2DData = {}

---@param Size PVector2
---@param Center PVector2
function Rigidbody2DData:CreateBoxCollider(Size,Center)
	return dll.Rigidbody2D_CreateBoxCollider(self.thisID,Size,Center)
end

---@param Radius number
---@param Center PVector2
function Rigidbody2DData:CreateCircleCollider(Radius,Center)
	return dll.Rigidbody2D_CreateCircleCollider(self.thisID,Radius,Center)
end

---@param MotionType MotionType
function Rigidbody2DData:SetMotionType(MotionType)
	return dll.Rigidbody2D_SetMotionType(self.thisID,MotionType)
end

---@param position PVector3
function Rigidbody2DData:SetPosition(position)
	return dll.Rigidbody2D_SetPosition(self.thisID,position)
end

---@param rotation PVector3
function Rigidbody2DData:SetRotation(rotation)
	return dll.Rigidbody2D_SetRotation(self.thisID,rotation)
end

---@param velocity PVector3
function Rigidbody2DData:SetVelocity(velocity)
	return dll.Rigidbody2D_SetVelocity(self.thisID,velocity)
end

---@param Gravity number
function Rigidbody2DData:SetGravity(Gravity)
	return dll.Rigidbody2D_SetGravity(self.thisID,Gravity)
end

---@param Restitution number
function Rigidbody2DData:SetRestitution(Restitution)
	return dll.Rigidbody2D_SetRestitution(self.thisID,Restitution)
end

---@param Friction number
function Rigidbody2DData:SetFriction(Friction)
	return dll.Rigidbody2D_SetFriction(self.thisID,Friction)
end

---@param LinearDamping number
function Rigidbody2DData:SetLinearDamping(LinearDamping)
	return dll.Rigidbody2D_SetLinearDamping(self.thisID,LinearDamping)
end

---@param Sensor boolean
function Rigidbody2DData:SetSensor(Sensor)
	return dll.Rigidbody2D_SetSensor(self.thisID,Sensor)
end

---@param Active boolean
function Rigidbody2DData:SetActive(Active)
	return dll.Rigidbody2D_SetActive(self.thisID,Active)
end

---@param LayerName string
function Rigidbody2DData:SetLayer(LayerName)
	return dll.Rigidbody2D_SetLayer(self.thisID,LayerName)
end

---@return string
function Rigidbody2DData:GetLayer()
	return ffi.string(dll.Rigidbody2D_GetLayer(self.thisID))
end

local Rigidbody2D_mt = {
	__index = Rigidbody2DData
}

ffi.metatype("Rigidbody2DData", Rigidbody2D_mt)

Rigidbody2D = Rigidbody2D or {}

---@param ID number 
---@return Rigidbody2DData 
function Rigidbody2D.Add(ID) 
	local data = dll.Rigidbody2D_Add(ID)
	data.thisID = ID
	return data 
end 

---@param ID number 
---@return Rigidbody2DData 
function Rigidbody2D.Get(ID) 
	return dll.Rigidbody2D_Get(ID) 
end 

---@param ID number 
---@return boolean 
function Rigidbody2D.Has(ID) 
	return dll.Rigidbody2D_Has(ID) 
end 

return Rigidbody2D