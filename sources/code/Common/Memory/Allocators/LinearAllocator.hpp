#pragma once

#include <stdint.h>

namespace Grindstone::Allocators {
	/**
	 * \brief A linear allocator where memory can only be deallocated when freed.
	 *
	 * Memory in a linear allocator is allocated sequentially, similar to a StackAllocator,
	 * one after the other. This removes the possibility of fragmentation, but with LinearAllocators
	 * specifically, they are never deallocated. Also known as an arena allocator.
	 */
	class LinearAllocator {
	public:
		~LinearAllocator();

		void Initialize(void* ownedMemory, size_t size);
		bool Initialize(size_t size);
		void* Allocate(size_t size);
		void Clear();
		void ClearAndZero();
		void Destroy();

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

	private:
		size_t totalMemorySize;
		size_t usedSize;

		void* memory;
		bool hasAllocatedOwnMemory;
	};
}
