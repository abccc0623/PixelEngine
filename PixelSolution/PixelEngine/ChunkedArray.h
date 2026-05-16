#pragma once
#include <vector>
#include <array>
#include <queue>
#include <memory>

namespace ECS
{
	struct ChunkedID
	{
		unsigned int value;
		ChunkedID() = default;
		ChunkedID(unsigned int index, unsigned int version)
		{
			value = ((version << 20) | (index & 0xFFFFF));
		}
		ChunkedID(unsigned int value)
		{
			this->value = value;
		}
		constexpr unsigned int Index() const { return value & 0xFFFFF; }
		constexpr unsigned int Version() const { return (value >> 20) & 0xFFF; }
		bool operator==(const ChunkedID& other) const { return value == other.value; }
		bool operator!=(const ChunkedID& other) const { return value != other.value; }
	};

	template <typename T, int chunksMaxSize = 1024>
	class ChunkedArray
	{
	public:
		T* Get(ECS::ChunkedID id)
		{
			unsigned int index = id.Index();
			unsigned int version = id.Version();

			size_t chunkIndex = index / chunksMaxSize;
			size_t localIndex = index % chunksMaxSize;
			return &(*chunks[chunkIndex])[localIndex]; // 포인터 100% 안전 보장!
		}

		ChunkedID Add()
		{
			ChunkedID id;
			if (freeSlots.size() != 0)
			{
				//빈곳이 있다면 그곳에 데이터 넣기
				int outIndex = freeSlots.front();
				freeSlots.pop();
				return ChunkedID(outIndex, versions[outIndex]);
			}

			//빈곳이 없다면 뒤부터 순서대로
			if (currentSize >= chunks.size() * chunksMaxSize)
			{
				chunks.push_back(std::make_unique<std::array<T, chunksMaxSize>>());
			}
			unsigned int chunkIndex = currentSize / chunksMaxSize;
			unsigned int localIndex = currentSize % chunksMaxSize;
			currentSize++;
			versions.push_back(0);
			SetLife(currentSize - 1, true);
			return ChunkedID(currentSize - 1, 0);
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
			unsigned int version = id.Version();

			auto slot = Get(id);
			if (slot)
			{
				*slot = T();
			}
			freeSlots.push(index);
			versions[index]++;
			SetLife(index, false);
		}

		void SetLife(unsigned int index, bool isLife)
		{
			if (isLife == true)
			{
				if (index >= activeSlots.size()) activeSlots.resize(index + 1, false);
				activeSlots[index] = true;
			}
			else
			{
				if (index < activeSlots.size()) activeSlots[index] = false;
			}
		}

		template <typename Func>
		void ForEach(Func callback)
		{
			for (size_t i = 0; i < currentSize; ++i)
			{
				// 빈 방(구멍)이면 빛의 속도로 다음 방으로 건너뜀!
				if (!activeSlots[i]) continue;

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
		std::vector<bool> activeSlots;
		int currentSize;
	};
}

