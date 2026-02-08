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

struct RenderingData
{
	float World[16];
	ObjectID Model_ID = -1;
	ObjectID Texture_ID = -1;
	RENDER_TYPE Type = RENDER_TYPE::NONE;
	CameraOption cameraOption;
};
