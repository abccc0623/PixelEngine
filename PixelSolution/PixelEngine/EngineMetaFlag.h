#pragma once

namespace EngineMetaFlag
{
	constexpr long LuaBind = 1 << 1;

	constexpr long Class = 1 << 2;
	constexpr long ClassData = 1 << 3;

	constexpr long Component = 1 << 4;
	constexpr long ComponentData = 1 << 5;

	constexpr long StaticGlobal = 1 << 11;
	constexpr long Enum = 1 << 12;
}
