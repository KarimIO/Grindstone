#pragma once

#include <Common/Memory/Allocators/DynamicAllocator.hpp>
#include <Common/String.hpp>

namespace Grindstone::Memory {
	class AllocatorCore {
	public:
		bool Initialize(size_t sizeInMegs);
		StringRef AllocateString(size_t size);
		StringRef AllocateString(Grindstone::StringRef srcString);

		void* Allocate(size_t size);
		bool Free(void* memPtr);

		virtual size_t GetUsed() const;
		virtual size_t GetTotal() const;

		virtual bool IsCleared() const;

		template<typename T, typename... Args>
		T* Allocate(Args&&... params) {
			T* ptr = static_cast<T*>(allocator.Allocate(sizeof(T)));
			if (ptr != nullptr) {
				// Call the constructor on the newly allocated memory
				new (ptr) T(std::forward<Args>(params)...);
			}

			return ptr;
		}

		template<typename T>
		T* AllocateWithoutConstructor() {
			return static_cast<T*>(allocator.Allocate(sizeof(T)));
		}

	private:
		Allocators::DynamicAllocator allocator;
	};
}
