#include "pch.h"
#include "FontFactory.h"
#include "GraphicsCore.h"
#include "ResourceManager.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace
{
	constexpr int FontAtlasWidth = 4096;
	constexpr int FontAtlasHeight = 4096;
	constexpr float DefaultFontSize = 32.0f;
	constexpr int AsciiFirstCodepoint = 32;
	constexpr int AsciiCharacterCount = 95;
	constexpr int KoreanJamoFirstCodepoint = 0x3131;
	constexpr int KoreanJamoCharacterCount = 94;
	constexpr int KoreanFirstCodepoint = 0xAC00;
	constexpr int KoreanCharacterCount = 11172;
}

bool PixelGraphics::FontFactory::Initialize(GraphicsCore* graphicsCore)
{
	Clear();
	this->graphicsCore = graphicsCore;
	defaultFontKey = static_cast<std::uint16_t>(ResourceDefaultKey::FONT);
	return this->graphicsCore != nullptr && LoadDefaultFont();
}

void PixelGraphics::FontFactory::Release()
{
	Clear();
	graphicsCore = nullptr;
}

void PixelGraphics::FontFactory::Clear()
{
	fonts.clear();
	defaultFontFileData.clear();
	nextFontKey = 1;
}

std::uint16_t PixelGraphics::FontFactory::Load(const std::string& path)
{
	for (auto& [key, font] : fonts)
	{
		if (font.path == path)
		{
			return key;
		}
	}

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
	{
		return defaultFontKey;
	}

	std::streamsize fileSize = file.tellg();
	if (fileSize <= 0)
	{
		return defaultFontKey;
	}
	file.seekg(0, std::ios::beg);

	std::vector<unsigned char> fontFileData(static_cast<size_t>(fileSize));
	if (!file.read(reinterpret_cast<char*>(fontFileData.data()), fileSize))
	{
		return defaultFontKey;
	}

	const std::uint16_t key = AllocateKey();
	constexpr std::uint16_t invalidKey = (std::numeric_limits<std::uint16_t>::max)();
	if (key == invalidKey || !CreateFontResource(fontFileData, key, path))
	{
		return defaultFontKey;
	}
	return key;
}

bool PixelGraphics::FontFactory::LoadDefaultFont()
{
	const HMODULE module = GetPixelGraphicsModule();
	const HRSRC resource = FindResourceW(module, L"DEFAULT_FONT", L"TTF");
	if (!resource)
	{
		return false;
	}

	const HGLOBAL resourceData = LoadResource(module, resource);
	if (!resourceData)
	{
		return false;
	}

	const void* bytes = LockResource(resourceData);
	const DWORD byteCount = SizeofResource(module, resource);
	if (!bytes || byteCount == 0)
	{
		return false;
	}

	const unsigned char* begin = static_cast<const unsigned char*>(bytes);
	defaultFontFileData.assign(begin, begin + byteCount);
	return CreateFontResource(defaultFontFileData, defaultFontKey, "DEFAULT_FONT");
}

bool PixelGraphics::FontFactory::CreateFontResource(const std::vector<unsigned char>& fontFileData, std::uint16_t key, const std::string& path)
{
	if (!graphicsCore || !graphicsCore->GetDevice() || fontFileData.empty())
	{
		return false;
	}

	const int fontOffset = stbtt_GetFontOffsetForIndex(fontFileData.data(), 0);
	stbtt_fontinfo fontInfo = {};
	if (fontOffset < 0 || !stbtt_InitFont(&fontInfo, fontFileData.data(), fontOffset))
	{
		return false;
	}

	std::vector<unsigned char> atlasAlpha(FontAtlasWidth * FontAtlasHeight);
	std::vector<stbtt_packedchar> asciiCharacters(AsciiCharacterCount);
	std::vector<stbtt_packedchar> koreanJamoCharacters(KoreanJamoCharacterCount);
	std::vector<stbtt_packedchar> koreanCharacters(KoreanCharacterCount);

	stbtt_pack_range ranges[3] = {};
	ranges[0].font_size = DefaultFontSize;
	ranges[0].first_unicode_codepoint_in_range = AsciiFirstCodepoint;
	ranges[0].num_chars = AsciiCharacterCount;
	ranges[0].chardata_for_range = asciiCharacters.data();
	ranges[1].font_size = DefaultFontSize;
	ranges[1].first_unicode_codepoint_in_range = KoreanJamoFirstCodepoint;
	ranges[1].num_chars = KoreanJamoCharacterCount;
	ranges[1].chardata_for_range = koreanJamoCharacters.data();
	ranges[2].font_size = DefaultFontSize;
	ranges[2].first_unicode_codepoint_in_range = KoreanFirstCodepoint;
	ranges[2].num_chars = KoreanCharacterCount;
	ranges[2].chardata_for_range = koreanCharacters.data();

	stbtt_pack_context packContext = {};
	if (!stbtt_PackBegin(&packContext, atlasAlpha.data(), FontAtlasWidth, FontAtlasHeight, 0, 1, nullptr))
	{
		return false;
	}
	stbtt_PackSetOversampling(&packContext, 1, 1);
	const int packResult = stbtt_PackFontRanges(&packContext, fontFileData.data(), 0, ranges, 3);
	stbtt_PackEnd(&packContext);
	if (!packResult)
	{
		return false;
	}

	std::vector<unsigned char> atlasRGBA(FontAtlasWidth * FontAtlasHeight * 4);
	for (size_t index = 0; index < atlasAlpha.size(); ++index)
	{
		const unsigned char alpha = atlasAlpha[index];
		atlasRGBA[index * 4 + 0] = 255;
		atlasRGBA[index * 4 + 1] = 255;
		atlasRGBA[index * 4 + 2] = 255;
		atlasRGBA[index * 4 + 3] = alpha;
	}

	D3D11_TEXTURE2D_DESC textureDescription = {};
	textureDescription.Width = FontAtlasWidth;
	textureDescription.Height = FontAtlasHeight;
	textureDescription.MipLevels = 1;
	textureDescription.ArraySize = 1;
	textureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDescription.SampleDesc.Count = 1;
	textureDescription.Usage = D3D11_USAGE_IMMUTABLE;
	textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initialData = {};
	initialData.pSysMem = atlasRGBA.data();
	initialData.SysMemPitch = FontAtlasWidth * 4;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> atlasTexture;
	if (FAILED(graphicsCore->GetDevice()->CreateTexture2D(&textureDescription, &initialData, atlasTexture.GetAddressOf())))
	{
		return false;
	}

	FontResources fontResource = {};
	fontResource.key = key;
	fontResource.path = path;
	fontResource.fontSize = DefaultFontSize;
	if (FAILED(graphicsCore->GetDevice()->CreateShaderResourceView(atlasTexture.Get(), nullptr, fontResource.atlasTexture.GetAddressOf())))
	{
		return false;
	}

	int ascent = 0;
	int descent = 0;
	int lineGap = 0;
	stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
	const float fontScale = stbtt_ScaleForPixelHeight(&fontInfo, DefaultFontSize);
	fontResource.ascent = ascent * fontScale;
	fontResource.descent = descent * fontScale;
	fontResource.lineGap = lineGap * fontScale;
	fontResource.lineHeight = (ascent - descent + lineGap) * fontScale;

	auto addGlyphRange = [&fontResource](const std::vector<stbtt_packedchar>& characters, int firstCodepoint)
		{
			for (size_t index = 0; index < characters.size(); ++index)
			{
				const stbtt_packedchar& source = characters[index];
				GlyphData glyph = {};
				glyph.u0 = static_cast<float>(source.x0) / FontAtlasWidth;
				glyph.v0 = static_cast<float>(source.y0) / FontAtlasHeight;
				glyph.u1 = static_cast<float>(source.x1) / FontAtlasWidth;
				glyph.v1 = static_cast<float>(source.y1) / FontAtlasHeight;
				glyph.width = static_cast<float>(source.x1 - source.x0);
				glyph.height = static_cast<float>(source.y1 - source.y0);
				glyph.offsetX = source.xoff;
				glyph.offsetY = source.yoff;
				glyph.advance = source.xadvance;
				fontResource.glyphs.emplace(static_cast<char32_t>(firstCodepoint + index), glyph);
			}
		};

	addGlyphRange(asciiCharacters, AsciiFirstCodepoint);
	addGlyphRange(koreanJamoCharacters, KoreanJamoFirstCodepoint);
	addGlyphRange(koreanCharacters, KoreanFirstCodepoint);
	fonts.emplace(key, std::move(fontResource));
	return true;
}

std::uint16_t PixelGraphics::FontFactory::AllocateKey()
{
	constexpr std::uint16_t invalidKey = (std::numeric_limits<std::uint16_t>::max)();
	for (std::uint32_t count = 0; count < invalidKey - 1; ++count)
	{
		if (nextFontKey == defaultFontKey || nextFontKey == invalidKey)
		{
			nextFontKey = 1;
		}

		const std::uint16_t key = nextFontKey++;
		if (fonts.find(key) == fonts.end())
		{
			return key;
		}
	}
	return invalidKey;
}

FontResources* PixelGraphics::FontFactory::Get(std::uint16_t key)
{
	const auto found = fonts.find(key);
	if (found != fonts.end())
	{
		return &found->second;
	}

	const auto defaultFont = fonts.find(defaultFontKey);
	return defaultFont != fonts.end() ? &defaultFont->second : nullptr;
}
