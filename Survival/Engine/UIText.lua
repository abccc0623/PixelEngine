local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

ffi.cdef[[
	typedef struct {
		unsigned int thisID;
	} UITextData;

	UITextData* UIText_Add(unsigned int ID);
	UITextData* UIText_Get(unsigned int ID);
	bool UIText_Has(unsigned int ID);
	void UIText_SetText(unsigned int ID,const char* Text);
	void UIText_SetTextSize(unsigned int ID,float Scale);
	void UIText_SetFont(unsigned int ID,const char* FontName);
	void UIText_SetColor(unsigned int ID,float R,float G,float B,float A);
]]

---@class UITextData
local UITextData = {}

---@param Text string
function UITextData:SetText(Text)
	return dll.UIText_SetText(self.thisID,Text)
end

---@param Scale number
function UITextData:SetTextSize(Scale)
	return dll.UIText_SetTextSize(self.thisID,Scale)
end

---@param FontName string
function UITextData:SetFont(FontName)
	return dll.UIText_SetFont(self.thisID,FontName)
end

---@param R number
---@param G number
---@param B number
---@param A number
function UITextData:SetColor(R,G,B,A)
	return dll.UIText_SetColor(self.thisID,R,G,B,A)
end

local UIText_mt = {
	__index = UITextData
}

ffi.metatype("UITextData", UIText_mt)

UIText = UIText or {}

---@param ID number 
---@return UITextData 
function UIText.Add(ID) 
	local data = dll.UIText_Add(ID)
	data.thisID = ID
	return data 
end 

---@param ID number 
---@return UITextData 
function UIText.Get(ID) 
	return dll.UIText_Get(ID) 
end 

---@param ID number 
---@return boolean 
function UIText.Has(ID) 
	return dll.UIText_Has(ID) 
end 

return UIText