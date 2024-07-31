#pragma once

#include <Common/Memory/Allocators/DynamicAllocator.hpp>
#include <Common/String.hpp>

namespace Grindstone::Memory::AllocatorCore {
	struct AllocatorState {
		Allocators::DynamicAllocator dynamicAllocator;
	};

	Grindstone::Memory::AllocatorCore::AllocatorState* GetAllocatorState();
	void SetAllocatorState(Grindstone::Memory::AllocatorCore::AllocatorState* newAllocatorState);

	bool Initialize(size_t sizeInMegs);
	void CloseAllocator();
	
	StringRef AllocateString(size_t size);
	StringRef AllocateString(Grindstone::StringRef srcString);

	bool FreeWithoutDestructor(void* memPtr);

	size_t GetUsed();
	size_t GetTotal();

	bool IsEmpty();

	void* AllocateRaw(size_t size, const char* debugName);

	template<typename T, typename... Args>
	UniquePtr<T> AllocateUnique(Args&&... params) {
		return static_cast<T*>(GetAllocatorState()->dynamicAllocator.AllocateUnique<T>(params));
	}

	template<typename T, typename... Args>
	SharedPtr<T> AllocateShared(Args&&... params) {
		return static_cast<T*>(GetAllocatorState()->dynamicAllocator.AllocateShared<T>(params));
	}

	template<typename T, typename... Args>
	T* Allocate(Args&&... params) {
		T* ptr = static_cast<T*>(GetAllocatorState()->dynamicAllocator.AllocateRaw(sizeof(T), typeid(T).name()));
		if (ptr != nullptr) {
			// Call the constructor on the newly allocated memory
			new (ptr) T(std::forward<Args>(params)...);
		}

		return ptr;
	}

	template<typename T>
	bool Free(T* memPtr, bool shouldClear = false) {
		bool retVal = GetAllocatorState()->dynamicAllocator.Free(memPtr, shouldClear);
		if (retVal) {
			reinterpret_cast<T*>(memPtr)->~T();
			return true;
		}

		return false;
	}
};
