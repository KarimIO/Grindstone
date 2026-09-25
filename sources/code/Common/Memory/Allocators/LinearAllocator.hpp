#pragma once
#include <utility>
#include <type_traits>

namespace Grindstone::Memory::Allocators {
	/**
	 * \brief A linear allocator where memory can only be deallocated when the entire allocator is cleared.
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
		void* AllocateRaw(size_t size, size_t alignment);
		void Clear();
		void ClearAndZero();
		void Destroy();

		template<typename T, typename... Args>
		T* Allocate(Args&&... params) {
			static_assert(std::is_constructible_v<T, Args...>, "Type T must be constructible with given arguments.");

			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), alignof(T)));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return ptr;
		}

		template<typename T>
		T* AllocateWithoutConstructor() {
			return static_cast<T*>(Allocate(sizeof(T), alignof(T)));
		}

		size_t GetUsedSize() const;

	private:
		size_t totalMemorySize;
		size_t usedSize;

		void* memory;
		bool hasAllocatedOwnMemory;
	};
}
