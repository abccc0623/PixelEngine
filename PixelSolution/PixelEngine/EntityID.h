#pragma once

namespace ECS
{
	struct EntityID
	{
		EntityID(unsigned int index, unsigned int version)
		{
			value = ((version << 20) | (index & 0xFFFFF));
		}
		EntityID(unsigned int id) { value = id; }
		unsigned int value;
		constexpr unsigned int Index() const { return value & 0xFFFFF; }
		constexpr unsigned int Version() const { return (value >> 20) & 0xFFF; }

		constexpr void SetValue(unsigned int index, unsigned int version)
		{
			value = ((version << 20) | (index & 0xFFFFF));
		}
		bool operator==(const EntityID& other) const { return value == other.value; }
		bool operator!=(const EntityID& other) const { return value != other.value; }
	};
};


