#pragma once
#include "KeyAllocator.h"
#include <type_traits>
#include <unordered_map>
#include <Windows.h>
#include <string>

namespace PixelGraphics
{
	class GraphicsCore;
	class ResourceFactory
	{
	public:
		ResourceFactory() = default;
		virtual ~ResourceFactory() = default;
	public:
		virtual bool Initialize(PixelGraphics::GraphicsCore* graphicsCore) = 0;
		virtual void Release() = 0;
		virtual void Clear() = 0;
		virtual std::uint16_t Load(const std::string& path) = 0;
	protected:
		static HMODULE GetPixelGraphicsModule();
	};
}

