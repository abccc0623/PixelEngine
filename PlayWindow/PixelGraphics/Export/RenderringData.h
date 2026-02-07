#pragma once
enum RENDER_TYPE : int
{
	NONE ,
	DEBUG,
	CAMERA,
	QUAD,
};



struct RenderingData
{
	float World[16];
	int ModelIndex = -1;
	RENDER_TYPE Type = RENDER_TYPE::NONE;
};