#include "pch.h"
#include "PixelMetaAPI.h"
#include "PValue.h"
#include "PType.h"
PValue::PValue()
{
	Data.v_raw = 0;
	Type = nullptr;
}

PValue::PValue(int32_t val)
{
	Data.v_int = val;
	Type = GetTypeByKeyword<int>();
}

PValue::PValue(float val)
{
	Data.v_float = val;
	Type = GetTypeByKeyword<float>();
}

PValue::PValue(bool val)
{
	Data.v_bool = val;
	Type = GetTypeByKeyword<bool>();
}

PValue::PValue(char val)
{
	Data.v_char = val;
	Type = GetTypeByKeyword<char>();
}

PValue::PValue(double val)
{
	Data.v_double = val;
	Type = GetTypeByKeyword<double>();
}

PValue::PValue(void* ptr)
{
	Data.v_ptr = ptr;
	Type = GetTypeByKeyword<void*>();
}

PValue::~PValue()
{
	Type = nullptr;
}

int32_t PValue::AsInt()
{

	return Data.v_int;
}

float PValue::AsFloat()
{
	if (Type == GetTypeByKeyword<float>())
	{
		return Data.v_float;
	}
	return 0.0f;
}

bool PValue::AsBool()
{
	if (Type == GetTypeByKeyword<bool>())
	{
		return Data.v_bool;
	}
	return false;
}

char PValue::AsChar()
{
	if (Type == GetTypeByKeyword<char>())
	{
		return Data.v_char;
	}
	return '\0';
}

double PValue::Asdouble()
{
	if (Type == GetTypeByKeyword<double>())
	{
		return Data.v_double;
	}
	return 0.0f;
}

std::string PValue::AsString()
{
	if (Type == GetTypeByKeyword<std::string>())
	{
		return value;
	}
}

void* PValue::AsPointer()
{
	return Data.v_ptr;
}
