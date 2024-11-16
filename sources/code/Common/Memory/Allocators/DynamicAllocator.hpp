#pragma once

#include <string>
#include <functional>
#include <stdint.h>
#include <map>
#include <utility>

#include "../SmartPointers.hpp"

namespace Grindstone::Memory::Allocators {
	/**
	 * \brief A dynamic allocator represented by a linked list.
	 *
	 * Memory in a dynamic allocator can be allocated and deallocated freely, with no restrictions.
	 * Allocations have a header that points to the next block of memory, and so free memory can be found
	 * in between allocated blocks of memory.
	 */
	class DynamicAllocator {
	public:
		struct FreeHeader {
			size_t blockSize;
			FreeHeader* nextFreeBlock = nullptr;
		};

		struct AllocationHeader {
			size_t blockSize;
			uint8_t padding;
		};

		enum class SearchPolicy {
			FirstSearch = 0,
			BestSearch
		};

		~DynamicAllocator();

		bool Initialize(size_t size);
		void Initialize(void* ownedMemory, size_t size);

		void* AllocateRaw(size_t size, size_t alignment, const char* debugName);
		bool Free(void* memPtr);

		bool IsEmpty() const;

		size_t GetTotalMemorySize() const;
		size_t GetPeakSize() const;
		size_t GetUsedSize() const;
		void* GetMemory() const;

		template<typename T, typename... Args>
		Grindstone::Memory::SmartPointers::SharedPtr<T> AllocateShared(Args&&... params) {
			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), alignof(T), typeid(T).name()));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return Grindstone::Memory::SmartPointers::SharedPtr<T>(ptr, deleterFn);
		}

		template<typename T, typename... Args>
		Grindstone::Memory::SmartPointers::UniquePtr<T> AllocateUnique(Args&&... params) {
			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), alignof(T), typeid(T).name()));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return Grindstone::Memory::SmartPointers::UniquePtr<T>(ptr, deleterFn);
		}

		template<typename T, typename... Args>
		T* AllocateRaw(Args&&... params) {
			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), alignof(T), typeid(T).name()));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return ptr;
		}

		template<typename T>
		bool Free(void* memPtr) {
			if (memPtr != nullptr) {
				return false;
			}

			reinterpret_cast<T*>(memPtr)->~T();
			Free(memPtr);

			return true;
		}

	private:
		void InitializeImpl(void* ownedMemory, size_t size);

		void* startMemory = nullptr;
		void* endMemory = nullptr;
		FreeHeader* firstFreeHeader = nullptr;

		std::function<void(void*)> deleterFn;
		size_t totalMemorySize = 0;
		size_t usedSize = 0;
		size_t peakSize = 0;
		SearchPolicy searchPolicy = SearchPolicy::BestSearch;
		bool shouldClear = false;
		bool hasAllocatedOwnMemory = false;

#ifdef _DEBUG
		std::map<void*, const char*> nameMap;
#endif
	};
}
