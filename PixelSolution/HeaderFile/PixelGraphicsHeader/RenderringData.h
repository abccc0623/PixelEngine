#pragma once
enum RENDER_TYPE : int
{
	NONE ,
	DEBUG,
	CAMERA,
	QUAD,
	BOX2D,
};

enum class ProjectionType
{
	Perspective, // 원근
	Orthographic // 직교
};

using ObjectID = size_t;
using Handle16 = unsigned short;
using Handle32 = unsigned int;
using Handle64 = unsigned long long;

struct SpriteData
{
	bool isAnimation;
	int MaxFramesX;
	int MaxFramesY;
	int FramesIndex;
};

struct CameraData
{
	ProjectionType Projection;
};

struct RenderingData
{
public:
	RENDER_TYPE Type = RENDER_TYPE::NONE;
	float World[16];

	Handle64 master_key		= 0;
	Handle16 mash_key		= 0;
	Handle16 material_key	= 0;
	Handle16 texture_key	= 0;
	union
	{
		SpriteData	sprite;
		CameraData	camera;
	};
	void Clear()
	{
		master_key = ~0ULL;
	}
	void Setting()
	{
		master_key |= (uint64_t)mash_key		<< 48;
		master_key |= (uint64_t)material_key	<< 32;
		master_key |= (uint64_t)texture_key		<< 16;
	}
};
