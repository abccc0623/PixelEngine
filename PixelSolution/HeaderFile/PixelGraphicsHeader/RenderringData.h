#pragma once
#include <cstdint>
enum class RENDER_TYPE : int
{
	NONE,
	DEBUG,
	LINE,
	CAMERA,
	QUAD,
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

using ObjectID = size_t;
using Handle32 = unsigned int;
using Handle64 = unsigned long long;

struct SpriteData
{
	bool isShared;
	float TilingX;
	float TilingY;
	float OffsetX;
	float OffsetY;
	int Order;
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
		SpriteData	sprite;
		SceneCameraData	camera;
		LineData	line;
	};
	void Clear()
	{
		mash_key = 0;
		material_key = 0;
		shader_key = 1;
		texture_key = 0;
	}
};
