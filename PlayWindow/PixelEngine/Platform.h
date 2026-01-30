#pragma once
class Platform
{
	public: 
		Platform() = default;
		virtual ~Platform() = default;
	private:
		

	public:
		virtual void Initialize() = 0;
		virtual void Update() = 0;
		virtual void Release() = 0;
};

