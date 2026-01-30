#pragma once
class Platform;

class PixelEngine
{
public:
	PixelEngine() = default;
	~PixelEngine() = default;
	void Initialize();
	void Update();
	void Release();
private:
	Platform* targetPlatform = nullptr;
};

