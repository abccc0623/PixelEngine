#pragma once
#include <cstdint>
enum class RENDER_TYPE : int
{
	NONE,
	DEBUG,
	LINE,
	CAMERA,
	QUAD,
	TEXT,
};

enum class PASS_TYPE : int
{
	NONE,
	SCENE,
	UI,
};

enum class ProjectionType
{
	Perspective, // 원근
	Orthographic // 직교
};

struct SpriteData
{
	bool isShared;
	float TilingX;
	float TilingY;
	float OffsetX;
	float OffsetY;
	int Order;
	float color[4];
	float width;
	float height;
	float pivotX;
	float pivotY;
};

struct SceneCameraData
{
	ProjectionType Projection;
	float FovY;
	float NearZ;
	float FarZ;
	float ZoomLevel;
	float ViewportX;
	float ViewportY;
	float ViewportWidth;
	float ViewportHeight;
};

struct LineData
{
	int start[3];
	int end[3];
};

struct TextData
{
	char content[256];
	std::uint16_t fontKey;
	float scale;
	float color[4];
};


struct RenderingData
{
public:
	RENDER_TYPE renderType = RENDER_TYPE::NONE;
	PASS_TYPE passType = PASS_TYPE::NONE;
	float World[16];
	std::uint16_t mash_key = 0;
	std::uint16_t material_key = 0;
	std::uint16_t shader_key = 1;
	std::uint16_t texture_key = 0;
	union
	{
		SpriteData sprite;
		SceneCameraData	camera;
		LineData line;
		TextData text;
	};
	void Clear()
	{
		mash_key = 0;
		material_key = 0;
		shader_key = 1;
		texture_key = 0;
	}
};
