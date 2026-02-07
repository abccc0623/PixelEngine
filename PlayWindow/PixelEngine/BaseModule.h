#pragma once
class BaseModule
{
public:
	BaseModule();
	virtual ~BaseModule();


	size_t GetHashCode();
private:
	size_t hasCode;
};

