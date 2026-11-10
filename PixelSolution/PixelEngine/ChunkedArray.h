#pragma once
#include <vector>
#include <array>
#include <queue>
#include <memory>
#include <cstdint>
#include <limits>

namespace ECS
{
	struct ChunkedID
	{
		static constexpr unsigned int IndexMask = 0xFFFFF;
		static constexpr unsigned int InvalidValue = (std::numeric_limits<unsigned int>::max)();
		unsigned int value = InvalidValue;
		ChunkedID() = default;
		ChunkedID(unsigned int index, unsigned int version)
		{
			value = ((version << 20) | (index & IndexMask));
		}
		ChunkedID(unsigned int value)
		{
			this->value = value;
		}
		constexpr unsigned int Index() const { return value & IndexMask; }
		constexpr unsigned int Version() const { return (value >> 20) & 0xFFF; }
		bool operator==(const ChunkedID& other) const { return value == other.value; }
		bool operator!=(const ChunkedID& other) const { return value != other.value; }
	};

	template <typename T, int chunksMaxSize = 128>
	class ChunkedArray
	{
	public:
		T* Get(ECS::ChunkedID id)
		{
			unsigned int index = id.Index();
			unsigned int version = id.Version();

			// 잘못되었거나 이미 삭제된 ID로 Chunk 메모리에 접근하지 않도록 검사한다.
			if (index >= static_cast<unsigned int>(currentSize) ||
				index >= versions.size() ||
				index >= occupiedSlots.size() || !occupiedSlots[index] ||
				versions[index] != version)
			{
				return nullptr;
			}

			size_t chunkIndex = index / chunksMaxSize;
			size_t localIndex = index % chunksMaxSize;
			// 두 번째 Chunk로 넘어갈 때 할당 여부를 확인하지 않으면 즉시 크래시가 난다.
			if (chunkIndex >= chunks.size() || !chunks[chunkIndex])
			{
				return nullptr;
			}

			return &(*chunks[chunkIndex])[localIndex];
		}

		ChunkedID Add()
		{
			if (!freeSlots.empty())
			{
				const unsigned int index = freeSlots.front();
				freeSlots.pop();
				occupiedSlots[index] = true;
				activeSlots[index] = true;
				return ChunkedID(index, versions[index]);
			}

			// Do not mask an out-of-range index back onto an existing slot.
			if (static_cast<unsigned int>(currentSize) > ChunkedID::IndexMask)
			{
				return ChunkedID(ChunkedID::InvalidValue);
			}
			if (currentSize >= chunks.size() * chunksMaxSize)
			{
				chunks.push_back(std::make_unique<std::array<T, chunksMaxSize>>());
			}
			const unsigned int index = currentSize++;
			versions.push_back(0);
			occupiedSlots.push_back(true);
			activeSlots.push_back(true);
			return ChunkedID(index, 0);
		}
		size_t GetSize() const
		{
			return currentSize;
		}
		size_t GetCapacity() const
		{
			return chunks.size() * chunksMaxSize;
		}

		void Remove(ECS::ChunkedID id)
		{
			unsigned int index = id.Index();

			auto slot = Get(id);
			// 중복 삭제 또는 잘못된 ID라면 freeSlots에 다시 등록하지 않는다.
			if (slot == nullptr)
			{
				return;
			}

			*slot = T();
			occupiedSlots[index] = false;
			activeSlots[index] = false;
			// Generation 255 is the last lifetime of this slot. Retire it rather
			// than wrapping back to an ID that a caller may still have saved.
			if (versions[index] < (std::numeric_limits<uint8_t>::max)())
			{
				++versions[index];
				freeSlots.push(index);
			}
		}

		void SetLife(unsigned int index, bool isLife)
		{
			// Activation is independent of ownership: pooled objects remain valid.
			if (index >= occupiedSlots.size() || !occupiedSlots[index]) return;
			activeSlots[index] = isLife;
		}

		template <typename Func>
		void ForEach(Func callback)
		{
			for (size_t i = 0; i < currentSize; ++i)
			{
				// 빈 방(구멍)이면 빛의 속도로 다음 방으로 건너뜀!
				if (!occupiedSlots[i] || !activeSlots[i])
				{
					continue;
				}

				size_t chunkIndex = i / chunksMaxSize;
				size_t localIndex = i % chunksMaxSize;

				// 살아있는 진짜 데이터만 콜백 함수에 밀어 넣습니다.
				callback(&(*chunks[chunkIndex])[localIndex], i);
			}
		}
	private:
		std::vector<std::unique_ptr<std::array<T, chunksMaxSize>>> chunks;
		std::vector<uint8_t> versions;
		std::queue<unsigned int> freeSlots;
		std::vector<bool> occupiedSlots;
		std::vector<bool> activeSlots;
		int currentSize = 0;
	};
}

