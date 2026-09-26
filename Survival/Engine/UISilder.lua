local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

ffi.cdef[[
	typedef struct {
		unsigned int thisID;
		float minValue;
		float maxValue;
		float value;
		float width;
		float height;
	} UISilderData;

	UISilderData* UISilder_Add(unsigned int ID);
	UISilderData* UISilder_Get(unsigned int ID);
	bool UISilder_Has(unsigned int ID);
	void UISilder_SetRange(unsigned int ID,float MinValue,float MaxValue);
	void UISilder_SetValue(unsigned int ID,float Value);
	void UISilder_SetSize(unsigned int ID,float Width,float Height);
	void UISilder_SetBackgroundColor(unsigned int ID,float R,float G,float B,float A);
	void UISilder_SetFillColor(unsigned int ID,float R,float G,float B,float A);
	void UISilder_SetBackgroundTexture(unsigned int ID,const char* TextureName);
	void UISilder_SetFillTexture(unsigned int ID,const char* TextureName);
	void UISilder_SetOrder(unsigned int ID,int Order);
]]

---@class UISilderData
---@field minValue number
---@field maxValue number
---@field value number
---@field width number
---@field height number
local UISilderData = {}

---@param MinValue number
---@param MaxValue number
function UISilderData:SetRange(MinValue,MaxValue)
	return dll.UISilder_SetRange(self.thisID,MinValue,MaxValue)
end

---@param Value number
function UISilderData:SetValue(Value)
	return dll.UISilder_SetValue(self.thisID,Value)
end

---@param Width number
---@param Height number
function UISilderData:SetSize(Width,Height)
	return dll.UISilder_SetSize(self.thisID,Width,Height)
end

---@param R number
---@param G number
---@param B number
---@param A number
function UISilderData:SetBackgroundColor(R,G,B,A)
	return dll.UISilder_SetBackgroundColor(self.thisID,R,G,B,A)
end

---@param R number
---@param G number
---@param B number
---@param A number
function UISilderData:SetFillColor(R,G,B,A)
	return dll.UISilder_SetFillColor(self.thisID,R,G,B,A)
end

---@param TextureName string
function UISilderData:SetBackgroundTexture(TextureName)
	return dll.UISilder_SetBackgroundTexture(self.thisID,TextureName)
end

---@param TextureName string
function UISilderData:SetFillTexture(TextureName)
	return dll.UISilder_SetFillTexture(self.thisID,TextureName)
end

---@param Order number
function UISilderData:SetOrder(Order)
	return dll.UISilder_SetOrder(self.thisID,Order)
end

local UISilder_mt = {
	__index = UISilderData
}

ffi.metatype("UISilderData", UISilder_mt)

UISilder = UISilder or {}

---@param ID number 
---@return UISilderData 
function UISilder.Add(ID) 
	local data = dll.UISilder_Add(ID)
	data.thisID = ID
	return data 
end 

---@param ID number 
---@return UISilderData 
function UISilder.Get(ID) 
	return dll.UISilder_Get(ID) 
end 

---@param ID number 
---@return boolean 
function UISilder.Has(ID) 
	return dll.UISilder_Has(ID) 
end 

return UISilder