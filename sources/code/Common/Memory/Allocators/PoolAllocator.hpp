#pragma once

#include <stdint.h>
#include <utility>
#include <functional>
#include <memory>

#include "../SmartPointers.hpp"

namespace Grindstone::Allocators {
	class BasePoolAllocator {
	public:

		void Clear();
		void ClearAndZero();
		void Destroy();

		bool IsEmpty() const;
		size_t GetUsedCount() const;

		struct FreeLink {
			FreeLink* next;
		};

	protected:
		BasePoolAllocator() = default;

		inline void* AllocateImpl();
		inline void SetupLinkedList();

		void DeallocateImpl(size_t index);
		void DeallocateImpl(void* ptr);

		// Total Memory size is used in case we are passed owned memory that is bigger than totalChunkCount * chunkSize
		size_t totalMemorySize = 0;
		size_t chunkSize = 0;
		size_t usedChunkCount = 0;
		size_t totalChunkCount = 0;

		FreeLink* headFreePtr = nullptr;
		void* memory = nullptr;
		bool hasAllocatedOwnMemory = false;
		std::function<void(void*)> deleteFn;
	};

	/**
	 * \brief A generic pool allocator where all allocations have up to a certain chunk size.
	 *
	 * Memory in a linear allocator is allocated sequentially, similar to a StackAllocator,
	 * one after the other. This removes the possibility of fragmentation, but with LinearAllocators
	 * specifically, they are never deallocated. Also known as an arena allocator.
	 */
	class GenericPoolAllocator : public BasePoolAllocator {
	public:
		GenericPoolAllocator() = default;
		~GenericPoolAllocator();

		void Initialize(void* ownedMemory, size_t totalSize, size_t sizePerChunk);
		bool Initialize(size_t sizePerChunk, size_t maxChunkCount);
		void* Allocate();
		void Deallocate(size_t index);
		void Deallocate(void* ptr);

	};

	/**
	 * \brief A typed pool allocator where all allocations are the size of the type T.
	 *
	 * Memory in a linear allocator is allocated sequentially, similar to a StackAllocator,
	 * one after the other. This removes the possibility of fragmentation, but with LinearAllocators
	 * specifically, they are never deallocated. Also known as an arena allocator.
	 */
	template <typename T>
	class PoolAllocator : public BasePoolAllocator {
	public:
		PoolAllocator() = default;

		bool Initialize(void* ownedMemory, size_t totalSize) {
			memory = ownedMemory;
			totalMemorySize = totalSize;
			chunkSize = sizeof(T);
			usedChunkCount = 0;
			totalChunkCount = totalSize / chunkSize;
			hasAllocatedOwnMemory = false;

			deleteFn = [this](void* ptr) -> void {
				DeallocateImpl(ptr);
			};

			SetupLinkedList();
		}

		bool Initialize(size_t maxChunkCount) {
			chunkSize = sizeof(T);
			totalMemorySize = chunkSize * maxChunkCount;
			usedChunkCount = 0;
			totalChunkCount = maxChunkCount;

			memory = malloc(totalMemorySize);
			hasAllocatedOwnMemory = true;

			SetupLinkedList();

			deleteFn = [this](void* ptr) -> void {
				DeallocateImpl(ptr);
			};

			return memory != nullptr;
		}

		template<typename... Args>
		SharedPtr<T> AllocateShared(Args&&... params) {
			T* ptr = static_cast<T*>(AllocateImpl());
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return SharedPtr<T>(ptr, deleteFn);
		}

		template<typename... Args>
		UniquePtr<T> AllocateUnique(Args&&... params) {
			T* ptr = static_cast<T*>(AllocateImpl());
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return UniquePtr<T>(ptr, deleteFn);
		}

		template<typename... Args>
		T* AllocateRaw(Args&&... params) {
			T* ptr = static_cast<T*>(AllocateImpl());
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return ptr;
		}

		T* AllocateWithoutConstructor() {
			return static_cast<T*>(AllocateImpl());
		}

		void Deallocate(size_t index) {
			size_t chunkOffset = index * chunkSize;
			void* ptr = reinterpret_cast<char*>(memory) + chunkOffset;
			reinterpret_cast<T*>(ptr)->~T();
			DeallocateImpl(ptr);
		}

		void Deallocate(void* ptr) {
			reinterpret_cast<T*>(ptr)->~T();
			DeallocateImpl(ptr);
		}

		void DeallocateWithoutDestructor(size_t index) {
			size_t chunkOffset = index * chunkSize;
			void* ptr = reinterpret_cast<char*>(memory) + chunkOffset;
			DeallocateImpl(ptr);
		}

		void DeallocateWithoutDestructor(void* ptr) {
			DeallocateImpl(ptr);
		}
	};
}
