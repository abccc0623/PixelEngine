local ffi = require("ffi")
local dll = ffi.load("PixelEngine")

---@class Asset
ffi.cdef[[
	void Asset_Import(const char* assetPath);
]]

---@class Asset
Asset = Asset or {}

---@param assetPath string
function Asset.Import(assetPath)
	return dll.Asset_Import(assetPath)
end
return Asset