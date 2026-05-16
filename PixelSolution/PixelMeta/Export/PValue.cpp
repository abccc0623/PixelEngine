#include "pch.h"
#include "PValue.h"
#include "PType.h"
#include "RSystem.h"

extern RSystem* System;
PValue::PValue()
{
	Data.v_raw = 0;
	Type = nullptr;
}

PValue::PValue(int32_t val)
{
	Data.v_int = val;
	Type = System->GetTypeByString("int");
}

PValue::PValue(float val)
{
	Data.v_float = val;
	Type = System->GetTypeByString("float");
}

PValue::PValue(bool val)
{
	Data.v_bool = val;
	Type = System->GetTypeByString("bool");
}

PValue::PValue(char val)
{
	Data.v_char = val;
	Type = System->GetTypeByString("char");
}

PValue::PValue(double val)
{
	Data.v_double = val;
	Type = System->GetTypeByString("double");
}

PValue::PValue(void* ptr)
{
	Data.v_ptr = ptr;
	Type = nullptr;
}
PValue::PValue(std::string ptr)
{
	value = ptr;
	Type = nullptr;
}

PValue::~PValue()
{
	Type = nullptr;
}

int PValue::AsInt()
{
	if (Type->GetHash() == RSystem::int_Type)
	{
		return Data.v_int;
	}
	else if (Type->GetHash() == RSystem::float_Type)
	{
		return (int)Data.v_float;
	}
	return 0;
}

float PValue::AsFloat()
{
	if (Type->GetHash() == RSystem::float_Type)
	{
		return Data.v_float;
	}
	else if (Type->GetHash() == RSystem::int_Type)
	{
		return (float)Data.v_int;
	}
	return 0.0f;
}

bool PValue::AsBool()
{
	if (Type->GetHash() == RSystem::bool_Type)
	{
		return Data.v_bool;
	}
	return false;
}

char PValue::AsChar()
{
	return Data.v_char;
}

double PValue::Asdouble()
{
	if (Type->GetHash() == RSystem::double_Type)
	{
		return Data.v_double;
	}
	else if (Type->GetHash() == RSystem::float_Type)
	{
		return Data.v_float;
	}
	else if (Type->GetHash() == RSystem::int_Type)
	{
		return Data.v_int;
	}
	return 0.0f;
}

std::string PValue::AsString()
{
	if (Type->GetHash() == RSystem::string_Type)
	{
		return value;
	}
	return value;
}

void* PValue::AsPointer()
{
	return Data.v_ptr;
}
