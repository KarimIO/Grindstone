#pragma once

#include <string>
#include <functional>
#include <stdint.h>
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
		struct Header {
			bool isAllocated;
			Header* nextHeader;
			Header* previousHeader;
#ifdef _DEBUG
			const char* debugName;
#endif
		};

		~DynamicAllocator();

		void Initialize(void* ownedMemory, size_t size);
		bool Initialize(size_t size);

		void* AllocateRaw(size_t size, const char* debugName);
		bool Free(void* memPtr, bool shouldClear = false);

		bool IsEmpty() const;

		size_t GetTotalMemorySize() const;
		size_t GetUsedSize() const;
		void* GetMemory() const;

		// Assumes that this is a header within the memory block, in order to be more optimal
		static Header* GetHeaderOfBlock(void* block);

		template<typename T, typename... Args>
		SharedPtr<T> AllocateShared(Args&&... params) {
			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), typeid(T).name()));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return SharedPtr<T>(ptr, deleterFn);
		}

		template<typename T, typename... Args>
		UniquePtr<T> AllocateUnique(Args&&... params) {
			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), typeid(T).name()));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return UniquePtr<T>(ptr, deleterFn);
		}

		template<typename T, typename... Args>
		T* AllocateRaw(Args&&... params) {
			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), typeid(T).name()));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return ptr;
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
		std::function<void(void*)> deleterFn;
		size_t totalMemorySize = 0;
		size_t usedSize = 0;
		bool hasAllocatedOwnMemory = false;
	};
}

std::string BreakBytes(size_t bytes);

void* ValidatePtr(void* block);
bool IsValid(void* block);
