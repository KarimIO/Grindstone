#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include <Common/Memory/SmartPointers.hpp>

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
		static const size_t DEBUG_NAME_SIZE = 128;

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
		void* ReallocRaw(void* ptr, size_t newSize, size_t alignment, const char* debugName);
		bool Free(void* memPtr);

		bool IsEmpty() const;

		size_t GetTotalMemorySize() const;
		size_t GetPeakSize() const;
		size_t GetUsedSize() const;
		void* GetMemory() const;

#ifdef _DEBUG
		const std::map<void*, char[DEBUG_NAME_SIZE]>& GetNameMap() const {
			return nameMap;
		}
#endif

		template<typename T>
		Grindstone::SharedPtr<T> MakeShared(T* ptr) {
			return Grindstone::SharedPtr<T>(ptr, deleterFn);
		}

		template<typename T>
		Grindstone::UniquePtr<T> MakeUnique(T* ptr) {
			return Grindstone::UniquePtr<T>(ptr, deleterFn);
		}

		template<typename T, typename... Args>
		Grindstone::SharedPtr<T> AllocateShared(Args&&... params) {
			static_assert(std::is_constructible_v<T, Args...>, "Type T must be constructible with given arguments.");

			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), alignof(T), typeid(T).name()));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return Grindstone::SharedPtr<T>(ptr, deleterFn);
		}

		template<typename T, typename... Args>
		Grindstone::UniquePtr<T> AllocateUnique(Args&&... params) {
			static_assert(std::is_constructible_v<T, Args...>, "Type T must be constructible with given arguments.");

			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), alignof(T), typeid(T).name()));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return Grindstone::UniquePtr<T>(ptr, deleterFn);
		}

		template<typename T, typename... Args>
		T* AllocateRaw(Args&&... params) {
			static_assert(std::is_constructible_v<T, Args...>, "Type T must be constructible with given arguments.");

			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T), alignof(T), typeid(T).name()));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return ptr;
		}

		template <typename T, typename... Args>
		T* AllocateArray(size_t count, const Args&... args) {
			static_assert(
				std::is_constructible_v<T, const Args&...>,
				"Type T must be constructible from the given arguments.");

			if (count == 0) {
				return nullptr;
			}

			if (count > (std::numeric_limits<size_t>::max)() / sizeof(T)) {
				return nullptr;
			}

			T* ptr = static_cast<T*>(AllocateRaw(sizeof(T) * count, alignof(T), typeid(T).name()));
			if (ptr == nullptr) {
				return nullptr;
			}

			if constexpr (sizeof...(Args) == 0) {
				try {
					std::uninitialized_value_construct_n(ptr, count);
				} catch (...) {
					Free(ptr);
					throw;
				}
			} else {
				size_t constructed = 0;
				try {
					for (; constructed < count; ++constructed) {
						new (ptr + constructed) T(args...);
					}
				} catch (...) {
					DestroyRange(ptr, constructed);
					Free(ptr);
					throw;
				}
			}

			return ptr;
		}

		template <typename T>
		void FreeArray(T* ptr, size_t count) {
			if (ptr == nullptr) {
				return;
			}

			DestroyRange(ptr, count);
			Free(ptr);
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
		template <typename T>
		static void DestroyRange(T* ptr, size_t count) {
			if constexpr (!std::is_trivially_destructible_v<T>) {
				for (size_t i = count; i-- > 0;) {
					ptr[i].~T();
				}
			}
		}

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
		std::map<void*, char[DEBUG_NAME_SIZE]> nameMap;
#endif
	};
}
