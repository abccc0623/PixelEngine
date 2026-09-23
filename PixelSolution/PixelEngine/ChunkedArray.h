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
			ChunkedID id;
			if (freeSlots.size() != 0)
			{
				//빈곳이 있다면 그곳에 데이터 넣기
				int outIndex = freeSlots.front();
				freeSlots.pop();
				SetLife(outIndex, true);
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

			auto slot = Get(id);
			// 중복 삭제 또는 잘못된 ID라면 freeSlots에 다시 등록하지 않는다.
			if (slot == nullptr)
			{
				return;
			}

			*slot = T();
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
				if (index < activeSlots.size())
				{
					activeSlots[index] = false;
				}
			}
		}

		template <typename Func>
		void ForEach(Func callback)
		{
			std::string name = typeid(T).name();
			for (size_t i = 0; i < currentSize; ++i)
			{
				// 빈 방(구멍)이면 빛의 속도로 다음 방으로 건너뜀!
				if (activeSlots[i] == false)
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
		std::vector<bool> activeSlots;
		int currentSize = 0;
	};
}

