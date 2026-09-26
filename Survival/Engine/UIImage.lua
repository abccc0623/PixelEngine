local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

ffi.cdef[[
	typedef struct {
		unsigned int thisID;
	} UIImageData;

	UIImageData* UIImage_Add(unsigned int ID);
	UIImageData* UIImage_Get(unsigned int ID);
	bool UIImage_Has(unsigned int ID);
	void UIImage_SetTexture(unsigned int ID,const char* textureName);
	void UIImage_SetTextureSize(unsigned int ID,float Width,float Height);
	bool UIImage_SetAspectFit(unsigned int ID,float SlotWidth,float SlotHeight);
	void UIImage_SetPivot(unsigned int ID,float X,float Y);
	void UIImage_SetTiling(unsigned int ID,float X,float Y);
	void UIImage_SetOffset(unsigned int ID,float X,float Y);
	void UIImage_SetOrder(unsigned int ID,int Order);
	void UIImage_SetColor(unsigned int ID,float R,float G,float B,float A);
]]

---@class UIImageData
local UIImageData = {}

---@param textureName string
function UIImageData:SetTexture(textureName)
	return dll.UIImage_SetTexture(self.thisID,textureName)
end

---@param Width number
---@param Height number
function UIImageData:SetTextureSize(Width,Height)
	return dll.UIImage_SetTextureSize(self.thisID,Width,Height)
end

---@param SlotWidth number
---@param SlotHeight number
---@return boolean
function UIImageData:SetAspectFit(SlotWidth,SlotHeight)
	return dll.UIImage_SetAspectFit(self.thisID,SlotWidth,SlotHeight)
end

---@param X number
---@param Y number
function UIImageData:SetPivot(X,Y)
	return dll.UIImage_SetPivot(self.thisID,X,Y)
end

---@param X number
---@param Y number
function UIImageData:SetTiling(X,Y)
	return dll.UIImage_SetTiling(self.thisID,X,Y)
end

---@param X number
---@param Y number
function UIImageData:SetOffset(X,Y)
	return dll.UIImage_SetOffset(self.thisID,X,Y)
end

---@param Order number
function UIImageData:SetOrder(Order)
	return dll.UIImage_SetOrder(self.thisID,Order)
end

---@param R number
---@param G number
---@param B number
---@param A number
function UIImageData:SetColor(R,G,B,A)
	return dll.UIImage_SetColor(self.thisID,R,G,B,A)
end

local UIImage_mt = {
	__index = UIImageData
}

ffi.metatype("UIImageData", UIImage_mt)

UIImage = UIImage or {}

---@param ID number 
---@return UIImageData 
function UIImage.Add(ID) 
	local data = dll.UIImage_Add(ID)
	data.thisID = ID
	return data 
end 

---@param ID number 
---@return UIImageData 
function UIImage.Get(ID) 
	return dll.UIImage_Get(ID) 
end 

---@param ID number 
---@return boolean 
function UIImage.Has(ID) 
	return dll.UIImage_Has(ID) 
end 

return UIImage