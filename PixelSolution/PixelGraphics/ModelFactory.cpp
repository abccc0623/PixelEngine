#include "pch.h"
#include "ModelFactory.h"

#include "GraphicsCore.h"
#include "ResourceManager.h"

bool PixelGraphics::ModelFactory::Initialize(GraphicsCore* core)
{
	Clear();
	graphicsCore = core;
	defaultModelKey = static_cast<std::uint16_t>(ResourceDefaultKey::MODEL_QUAD);

	auto quad = CreateQuad();
	auto box2D = Create2DBox();
	if (!quad || !box2D)
	{
		return false;
	}

	quad->key = defaultModelKey;
	quad->path = "Quad";
	models.emplace(quad->key, std::move(*quad));

	box2D->key = static_cast<std::uint16_t>(ResourceDefaultKey::MODEL_BOX2D);
	box2D->path = "Box2D";
	models.emplace(box2D->key, std::move(*box2D));
	return true;
}

void PixelGraphics::ModelFactory::Release()
{
	Clear();
	graphicsCore = nullptr;
}

void PixelGraphics::ModelFactory::Clear()
{
	models.clear();
}

std::uint16_t PixelGraphics::ModelFactory::Load(const std::string& name)
{
	for (const auto& [key, model] : models)
	{
		if (model.path == name)
		{
			return key;
		}
	}
	return defaultModelKey;
}

std::unique_ptr<DirectModel> PixelGraphics::ModelFactory::CreateQuad()
{
	StaticVertex vertices[4] = {};
	const std::uint32_t indices[6] = { 0, 1, 2, 0, 2, 3 };

	vertices[0].Pos = { -0.5f, 0.5f, 0.0f };
	vertices[0].UV = { 0.0f, 0.0f };
	vertices[1].Pos = { 0.5f, 0.5f, 0.0f };
	vertices[1].UV = { 1.0f, 0.0f };
	vertices[2].Pos = { 0.5f, -0.5f, 0.0f };
	vertices[2].UV = { 1.0f, 1.0f };
	vertices[3].Pos = { -0.5f, -0.5f, 0.0f };
	vertices[3].UV = { 0.0f, 1.0f };
	return CreateModelBuffer(vertices, 4, indices, 6);
}

std::unique_ptr<DirectModel> PixelGraphics::ModelFactory::Create2DBox()
{
	DebugVertex vertices[4] = {};
	const std::uint32_t indices[8] = { 0, 1, 1, 2, 2, 3, 3, 0 };

	vertices[0].Pos = { -0.5f, 0.5f, 0.0f };
	vertices[1].Pos = { 0.5f, 0.5f, 0.0f };
	vertices[2].Pos = { 0.5f, -0.5f, 0.0f };
	vertices[3].Pos = { -0.5f, -0.5f, 0.0f };
	for (auto& vertex : vertices)
	{
		vertex.Color = { 1.0f, 0.0f, 0.0f };
	}

	return CreateModelBuffer(vertices, 4, indices, 8);
}

DirectModel* PixelGraphics::ModelFactory::Get(std::uint16_t key)
{
	const auto found = models.find(key);
	if (found != models.end())
	{
		return &found->second;
	}

	const auto defaultModel = models.find(defaultModelKey);
	if (defaultModel != models.end())
	{
		return &defaultModel->second;
	}

	return nullptr;
}
