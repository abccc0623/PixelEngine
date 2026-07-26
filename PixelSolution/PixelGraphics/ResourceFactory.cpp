#include "pch.h"
#include "ResourceFactory.h"

HMODULE PixelGraphics::ResourceFactory::GetPixelGraphicsModule()
{
	HMODULE module = nullptr;
	GetModuleHandleExW(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCWSTR>(
			&ResourceFactory::GetPixelGraphicsModule),
		&module);

	return module;
}
