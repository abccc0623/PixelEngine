#include "pch.h"
#include "TextRenderer.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <string_view>
#include "CameraManager.h"
#include "CBufferResources.h"
#include "FontFactory.h"
#include "GraphicsCore.h"
#include "PixelResources.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"
#include "SimpleMath.h"

namespace PixelGraphics
{
	constexpr size_t MaxGlyphCount = 256;
	int HexValue(char32_t character)
	{
		if (character >= U'0' && character <= U'9') return static_cast<int>(character - U'0');
		if (character >= U'a' && character <= U'f') return static_cast<int>(character - U'a') + 10;
		if (character >= U'A' && character <= U'F') return static_cast<int>(character - U'A') + 10;
		return -1;
	}

	bool ParseColorTag(const std::u32string& text, size_t position, DirectX::XMFLOAT4& color)
	{
		constexpr std::u32string_view Prefix = U"<color=#";
		constexpr size_t TagLength = 15;
		if (position + TagLength > text.size() || text.compare(position, Prefix.size(), Prefix) != 0 || text[position + 14] != U'>')
		{
			return false;
		}

		int values[6] = {};
		for (size_t index = 0; index < 6; ++index)
		{
			values[index] = HexValue(text[position + Prefix.size() + index]);
			if (values[index] < 0) return false;
		}

		color.x = static_cast<float>(values[0] * 16 + values[1]) / 255.0f;
		color.y = static_cast<float>(values[2] * 16 + values[3]) / 255.0f;
		color.z = static_cast<float>(values[4] * 16 + values[5]) / 255.0f;
		return true;
	}

	std::u32string DecodeUtf8(const char* text)
	{
		std::u32string result;
		const auto* cursor = reinterpret_cast<const unsigned char*>(text);
		while (*cursor != 0)
		{
			char32_t codepoint = 0;
			int trailing = 0;
			if (*cursor < 0x80) { codepoint = *cursor++; result.push_back(codepoint); continue; }
			if ((*cursor & 0xE0) == 0xC0) { codepoint = *cursor & 0x1F; trailing = 1; }
			else if ((*cursor & 0xF0) == 0xE0) { codepoint = *cursor & 0x0F; trailing = 2; }
			else if ((*cursor & 0xF8) == 0xF0) { codepoint = *cursor & 0x07; trailing = 3; }
			else { ++cursor; continue; }
			++cursor;
			bool valid = true;
			for (int index = 0; index < trailing; ++index)
			{
				if ((cursor[index] & 0xC0) != 0x80) { valid = false; break; }
				codepoint = (codepoint << 6) | (cursor[index] & 0x3F);
			}
			if (!valid) { continue; }
			cursor += trailing;
			result.push_back(codepoint);
		}
		return result;
	}
}

PixelGraphics::TextRenderer::TextRenderer(GraphicsCore* graphicsCore, ResourceManager* resourceManager, CameraManager* camera)
	: core(graphicsCore), resources(resourceManager), cameraManager(camera)
{}

PixelGraphics::TextRenderer::~TextRenderer() = default;

bool PixelGraphics::TextRenderer::Initialize()
{
	initialized = BuildBuffers();
	return initialized;
}

bool PixelGraphics::TextRenderer::BuildBuffers()
{
	D3D11_BUFFER_DESC vertexDescription = {};
	vertexDescription.ByteWidth = static_cast<UINT>(sizeof(StaticVertex) * MaxGlyphCount * 4);
	vertexDescription.Usage = D3D11_USAGE_DYNAMIC;
	vertexDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(core->GetDevice()->CreateBuffer(&vertexDescription, nullptr, vertexBuffer.GetAddressOf()))) return false;

	std::vector<std::uint32_t> indices(MaxGlyphCount * 6);
	for (size_t glyph = 0; glyph < MaxGlyphCount; ++glyph)
	{
		const std::uint32_t vertex = static_cast<std::uint32_t>(glyph * 4);
		const size_t index = glyph * 6;
		indices[index + 0] = vertex + 0; indices[index + 1] = vertex + 1; indices[index + 2] = vertex + 2;
		indices[index + 3] = vertex + 0; indices[index + 4] = vertex + 2; indices[index + 5] = vertex + 3;
	}
	D3D11_BUFFER_DESC indexDescription = {};
	indexDescription.ByteWidth = static_cast<UINT>(sizeof(std::uint32_t) * indices.size());
	indexDescription.Usage = D3D11_USAGE_IMMUTABLE;
	indexDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	D3D11_SUBRESOURCE_DATA indexData = {}; indexData.pSysMem = indices.data();
	if (FAILED(core->GetDevice()->CreateBuffer(&indexDescription, &indexData, indexBuffer.GetAddressOf()))) return false;

	D3D11_RASTERIZER_DESC rasterizerDescription = {};
	rasterizerDescription.FillMode = D3D11_FILL_SOLID;
	rasterizerDescription.CullMode = D3D11_CULL_NONE;
	rasterizerDescription.DepthClipEnable = TRUE;
	if (FAILED(core->GetDevice()->CreateRasterizerState(&rasterizerDescription, rasterizerState.GetAddressOf()))) return false;
	D3D11_SAMPLER_DESC samplerDescription = {};
	samplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDescription.AddressU = samplerDescription.AddressV = samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;
	return SUCCEEDED(core->GetDevice()->CreateSamplerState(&samplerDescription, sampler.GetAddressOf()));
}

void PixelGraphics::TextRenderer::Draw(const RenderingData& data)
{
	if (!initialized && !Initialize()) return;
	auto context = core->GetDeviceContext();
	const float blendFactor[4] = {};
	context->OMSetBlendState(core->GetAlphaBlendState(), blendFactor, UINT_MAX);
	context->OMSetDepthStencilState(core->GetDepthDisabledState(), 0);
	DrawText(data);
}

void PixelGraphics::TextRenderer::DrawText(const RenderingData& data)
{
	auto fontFactory = resources->GetFactory<FontFactory>(ResourceType::FONT);
	auto font = fontFactory ? fontFactory->Get(data.text.fontKey) : nullptr;
	if (!font || data.text.content[0] == '\0') return;

	const float scale = data.text.scale > 0.0f ? data.text.scale : 1.0f;
	std::vector<StaticVertex> vertices;
	vertices.reserve(MaxGlyphCount * 4);
	float cursorX = 0.0f, cursorY = font->ascent * scale;
	const std::u32string text = DecodeUtf8(data.text.content);
	const DirectX::XMFLOAT4 defaultColor(data.text.color[0], data.text.color[1], data.text.color[2], data.text.color[3]);
	DirectX::XMFLOAT4 currentColor = defaultColor;
	std::vector<DirectX::XMFLOAT4> colorStack;
	for (size_t textIndex = 0; textIndex < text.size(); ++textIndex)
	{
		DirectX::XMFLOAT4 tagColor = currentColor;
		if (ParseColorTag(text, textIndex, tagColor))
		{
			colorStack.push_back(currentColor);
			currentColor = tagColor;
			textIndex += 14;
			continue;
		}
		if (text.compare(textIndex, 8, U"</color>") == 0)
		{
			currentColor = colorStack.empty() ? defaultColor : colorStack.back();
			if (!colorStack.empty()) colorStack.pop_back();
			textIndex += 7;
			continue;
		}

		const char32_t codepoint = text[textIndex];
		if (codepoint == U'\n') { cursorX = 0.0f; cursorY += font->lineHeight * scale; continue; }
		auto found = font->glyphs.find(codepoint);
		if (found == font->glyphs.end()) found = font->glyphs.find(U'?');
		if (found == font->glyphs.end() || vertices.size() / 4 >= MaxGlyphCount) continue;
		const auto& glyph = found->second;
		const float x0 = cursorX + glyph.offsetX * scale, y0 = cursorY + glyph.offsetY * scale;
		const float x1 = x0 + glyph.width * scale, y1 = y0 + glyph.height * scale;
		vertices.push_back({ {x0,y0,0},{glyph.u0,glyph.v0},currentColor }); vertices.push_back({ {x1,y0,0},{glyph.u1,glyph.v0},currentColor });
		vertices.push_back({ {x1,y1,0},{glyph.u1,glyph.v1},currentColor }); vertices.push_back({ {x0,y1,0},{glyph.u0,glyph.v1},currentColor });
		cursorX += glyph.advance * scale;
	}
	if (vertices.empty()) return;

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	auto context = core->GetDeviceContext();
	if (FAILED(context->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return;
	std::memcpy(mapped.pData, vertices.data(), vertices.size() * sizeof(StaticVertex)); context->Unmap(vertexBuffer.Get(), 0);

	DirectX::SimpleMath::Matrix world; std::memcpy(&world, data.World, sizeof(data.World));
	auto shaderFactory = resources->GetFactory<ShaderFactory>(ResourceType::SHADER);
	auto shader = shaderFactory ? shaderFactory->Get(static_cast<std::uint16_t>(ResourceDefaultKey::SHADER_STATIC)) : nullptr;
	auto objectBufferResource = shaderFactory ? shaderFactory->GetBuffer("ObjectBuffer") : nullptr;
	if (!shader || !objectBufferResource) return;
	ObjectBuffer objectData = {};
	objectData.world = world.Transpose();
	objectData.wvp = (world * cameraManager->GetProjUI()).Transpose();
	objectData.TexMatrix = DirectX::SimpleMath::Matrix::Identity.Transpose();
	objectData.Color = DirectX::SimpleMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	ID3D11Buffer* cb = objectBufferResource->buffer.Get();
	context->UpdateSubresource(cb, 0, nullptr, &objectData, 0, 0);
	UINT stride = sizeof(StaticVertex), offset = 0; ID3D11Buffer* vb = vertexBuffer.Get();
	ID3D11ShaderResourceView* atlas = font->atlasTexture.Get(); ID3D11SamplerState* samplerState = sampler.Get();
	context->IASetInputLayout(shader->mLayout); context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->RSSetState(rasterizerState.Get());
	context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->VSSetShader(shader->mVertexShader, nullptr, 0); context->VSSetConstantBuffers(1, 1, &cb);
	context->PSSetShader(shader->mPixelShader, nullptr, 0); context->PSSetConstantBuffers(1, 1, &cb); context->PSSetShaderResources(0, 1, &atlas);
	context->PSSetSamplers(0, 1, &samplerState);
	context->DrawIndexed(static_cast<UINT>((vertices.size() / 4) * 6), 0, 0);
}
