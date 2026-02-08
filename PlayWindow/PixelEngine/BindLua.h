#pragma once

template <typename T>
class BindLua
{
public:
	BindLua() = default;
	virtual ~BindLua() = default;

    static void RegisterLua() {
        static bool bDone = []() {T::RegisterLua();return true;}();
    }
};
