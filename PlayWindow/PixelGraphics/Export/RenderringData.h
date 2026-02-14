#pragma once
enum RENDER_TYPE : int
{
	NONE ,
	DEBUG,
	CAMERA,
	QUAD,
};


using ObjectID = size_t;
struct CameraOption
{
	enum class ProjectionType 
	{
		Perspective, // 원근
		Orthographic // 직교
	};
	ProjectionType Projection = ProjectionType::Perspective;
};

using Handle16 = unsigned short;
using Handle32 = unsigned int;
using Handle64 = unsigned long long;
struct RenderingData
{
public:
	bool changeTransform = false;
	float World[16];

	RENDER_TYPE Type = RENDER_TYPE::NONE;
	CameraOption cameraOption;

	Handle16 model_key = 0;
	Handle16 texture_key = 0;
	Handle16 shader_key = 0;
	Handle64 master_key = 0;

	void Clear()
	{
		model_key = 0;
		texture_key = 0;
		shader_key = 0;
		master_key = ~0ULL;
	}
};
