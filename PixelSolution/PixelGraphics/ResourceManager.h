#pragma once
#include <map>

#include "TextureFactory.h"
#include "ShaderFactory.h"
#include "RasterizerStateFactory.h"
#include "ModelFactory.h"
#include "MaterialFactory.h"
#include "ResourceFactory.h"
namespace PixelGraphics
{
	class GraphicsCore;

	enum class ResourceType : int
	{
		TEXTURE,
		SHADER,
		RASTERIZER_STATE,
		MODEL,
		MATERIAL,
	};
	enum class ResourceDefaultKey : uint16_t
	{
		TEXTURE = 0,
		SHADER_DEBUG = 0,
		SHADER_STATIC = 1,
		RASTERIZER_SOLID = 0,
		RASTERIZER_WIRE = 1,
		MODEL_QUAD = 0,
		MODEL_BOX2D = 1,
		MATERIAL_DEFAULT = 0,
	};

	class ResourceManager
	{
	public:
		bool Initialize(GraphicsCore* graphicsCore);
		void Release();
		void Clear();
		uint16_t Load(ResourceType type, const char* path);

		template<typename FactoryType>
		FactoryType* GetFactory(ResourceType type)
		{
			const auto found = factoryMap.find(type);
			if (found == factoryMap.end())
			{
				return nullptr;
			}
			return dynamic_cast<FactoryType*>(found->second);
		}
	private:
		std::map<ResourceType, ResourceFactory*> factoryMap;
	};
}

