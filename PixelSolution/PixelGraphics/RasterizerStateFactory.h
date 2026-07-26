#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "PixelResources.h"
#include "ResourceFactory.h"

namespace PixelGraphics
{
	class GraphicsCore;

	class RasterizerStateFactory : public ResourceFactory
	{
	public:
		bool Initialize(GraphicsCore* graphicsCore) override;
		void Release() override;
		void Clear() override;
		std::uint16_t Load(const std::string& name) override;
		RasterizerStateResources* Get(std::uint16_t key);

	private:
		bool CreateDefaultState(std::uint16_t key, const std::string& name, D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode);
		GraphicsCore* graphicsCore = nullptr;
		std::unordered_map<std::uint16_t, RasterizerStateResources> rasterizerStates;
	};
}
