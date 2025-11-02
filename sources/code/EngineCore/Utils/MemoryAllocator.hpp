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

	size_t GetPeak();
	size_t GetUsed();
	size_t GetTotal();

	bool IsEmpty();

	void* AllocateRaw(size_t size, size_t alignment, const char* debugName);

	template<typename T>
	Grindstone::SharedPtr<T> MakeShared(T* ptr) {
		return GetAllocatorState()->dynamicAllocator.MakeShared(ptr);
	}

	template<typename T>
	Grindstone::UniquePtr<T> MakeUnique(T* ptr) {
		return GetAllocatorState()->dynamicAllocator.MakeUnique(ptr);
	}

	template<typename T, typename... Args>
	Grindstone::UniquePtr<T> AllocateUnique(Args&&... params) {
		return GetAllocatorState()->dynamicAllocator.AllocateUnique<T>(std::forward<Args>(params)...);
	}

	template<typename T, typename... Args>
	Grindstone::SharedPtr<T> AllocateShared(Args&&... params) {
		return GetAllocatorState()->dynamicAllocator.AllocateShared<T>(std::forward<Args>(params)...);
	}

	template<typename T, typename... Args>
	T* Allocate(Args&&... params) {
		T* ptr = static_cast<T*>(GetAllocatorState()->dynamicAllocator.AllocateRaw(sizeof(T), alignof(T), typeid(T).name()));
		if (ptr != nullptr) {
			// Call the constructor on the newly allocated memory
			new (ptr) T(std::forward<Args>(params)...);
		}

		return ptr;
	}

	template<typename T>
	T* AllocateArray(size_t arraySize) {
		T* ptr = static_cast<T*>(GetAllocatorState()->dynamicAllocator.AllocateRaw(sizeof(T) * arraySize, alignof(T), typeid(T).name()));
		return ptr;
	}

	template<typename T, typename... Args>
	T* AllocateArray(size_t arraySize, Args&&... params) {
		T* ptr = static_cast<T*>(GetAllocatorState()->dynamicAllocator.AllocateRaw(sizeof(T) * arraySize, alignof(T), typeid(T).name()));
		if (ptr != nullptr) {
			// Call the constructor on the newly allocated memory
			for (size_t i = 0; i < arraySize; ++i) {
				new (ptr + i) T(std::forward<Args>(params)...);
			}
		}

		return ptr;
	}

	template<typename T>
	bool Free(T* memPtr) {
		if (memPtr == nullptr) {
			return false;
		}

		reinterpret_cast<T*>(memPtr)->~T();
		GetAllocatorState()->dynamicAllocator.Free(memPtr);

		return true;
	}
};
