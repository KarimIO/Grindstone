#pragma once

#include <Common/Memory/Allocators/DynamicAllocator.hpp>
#include <Common/String.hpp>

namespace Grindstone::Memory {
	class AllocatorCore {
	public:
		bool Initialize(size_t sizeInMegs);
		StringRef AllocateString(size_t size);
		StringRef AllocateString(Grindstone::StringRef srcString);

		bool FreeWithoutDestructor(void* memPtr);

		virtual size_t GetUsed() const;
		virtual size_t GetTotal() const;

		virtual bool IsEmpty() const;

		template<typename T, typename... Args>
		UniquePtr<T> AllocateUnique(Args&&... params) {
			return static_cast<T*>(allocator.AllocateUnique<T>(params));
		}

		template<typename T, typename... Args>
		SharedPtr<T> AllocateShared(Args&&... params) {
			return static_cast<T*>(allocator.AllocateShared<T>(params));
		}

		template<typename T, typename... Args>
		T* Allocate(Args&&... params) {
			T* ptr = static_cast<T*>(allocator.AllocateRaw(sizeof(T)));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return ptr;
		}

		template<typename T>
		bool Free(T* memPtr, bool shouldClear = false) {
			bool retVal = allocator.Free(memPtr, shouldClear);
			if (retVal) {
				reinterpret_cast<T*>(memPtr)->~T();
				return true;
			}

			return false;
		}

	private:
		Allocators::DynamicAllocator allocator;
	};
}
