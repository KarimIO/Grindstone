#pragma once

#include <string>
#include <stdint.h>
#include <utility>

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
		struct Header {
			bool isAllocated;
			Header* nextHeader;
			Header* previousHeader;
		};

		void Initialize(void* ownedMemory, size_t size);
		bool Initialize(size_t size);

		void* Allocate(size_t size);
		bool Free(void* memPtr, bool shouldClear = false);

		void PrintBlocks();

		size_t GetTotalMemorySize() const;
		size_t GetUsedSize() const;
		void* GetMemory() const;

		// Assumes that this is a header within the memory block, in order to be more optimal
		static Header* GetHeaderOfBlock(void* block);

		template<typename T, typename... Args>
		T* Allocate(Args&&... params) {
			T* ptr = static_cast<T*>(Allocate(sizeof(T)));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return ptr;
		}

		template<typename T>
		T* AllocateWithoutConstructor() {
			return static_cast<T*>(Allocate(sizeof(T)));
		}

		template<typename T>
		bool Free(void* memPtr, bool shouldClear = false) {
			bool retVal = Free(memPtr, shouldClear);
			if (retVal) {
				reinterpret_cast<T*>(memPtr)->~T();
				return true;
			}

			return false;
		}

	private:
		Header* FindAvailableHeader(size_t size) const;

		Header* rootHeader = nullptr;
		size_t totalMemorySize = 0;
		size_t usedSize = 0;
		bool hasAllocatedOwnMemory = false;
	};
}

std::string BreakBytes(size_t bytes);

void* ValidatePtr(void* block);
bool IsValid(void* block);
