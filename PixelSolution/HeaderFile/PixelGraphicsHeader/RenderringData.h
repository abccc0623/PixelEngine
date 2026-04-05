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

struct MeshData
{
	Handle16 model_key;
	Handle16 texture_key;
	Handle16 shader_key;

	float offsetX;
	float offsetY;
	float tilingX;
	float tilingY;
	bool  isSpriteSheet;
	int   frameCount;
};

struct CameraData
{
	ProjectionType Projection;
};

struct BoxCollision2DData
{
	float sizeX;
	float sizeY;      
	float offsetX;  
	float offsetY;  
};

struct RenderingData
{
public:

	RENDER_TYPE Type = RENDER_TYPE::NONE;
	float World[16];
	Handle64 master_key = 0;
	union
	{
		MeshData mesh;
		CameraData camera;
		BoxCollision2DData collision;
	};
	void Clear()
	{
		master_key = ~0ULL;
	}
};
