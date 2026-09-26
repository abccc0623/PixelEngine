local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

ffi.cdef[[
	typedef struct {
		unsigned int thisID;
		float TilingX;
		float TilingY;
		float OffsetX;
		float OffsetY;
	} Renderer2DData;

	Renderer2DData* Renderer2D_Add(unsigned int ID);
	Renderer2DData* Renderer2D_Get(unsigned int ID);
	bool Renderer2D_Has(unsigned int ID);
	void Renderer2D_SetTexture(unsigned int ID,const char* textureName);
	void Renderer2D_SetColor(unsigned int ID,float R,float G,float B,float A);
]]

---@class Renderer2DData
---@field TilingX number
---@field TilingY number
---@field OffsetX number
---@field OffsetY number
local Renderer2DData = {}

---@param textureName string
function Renderer2DData:SetTexture(textureName)
	return dll.Renderer2D_SetTexture(self.thisID,textureName)
end

---@param R number
---@param G number
---@param B number
---@param A number
function Renderer2DData:SetColor(R,G,B,A)
	return dll.Renderer2D_SetColor(self.thisID,R,G,B,A)
end

local Renderer2D_mt = {
	__index = Renderer2DData
}

ffi.metatype("Renderer2DData", Renderer2D_mt)

Renderer2D = Renderer2D or {}

---@param ID number 
---@return Renderer2DData 
function Renderer2D.Add(ID) 
	local data = dll.Renderer2D_Add(ID)
	data.thisID = ID
	return data 
end 

---@param ID number 
---@return Renderer2DData 
function Renderer2D.Get(ID) 
	return dll.Renderer2D_Get(ID) 
end 

---@param ID number 
---@return boolean 
function Renderer2D.Has(ID) 
	return dll.Renderer2D_Has(ID) 
end 

return Renderer2D