#include <memory>

#include "LinearAllocator.hpp"

using namespace Grindstone::Allocators;

LinearAllocator::~LinearAllocator() {
	Destroy();
}

void LinearAllocator::Initialize(void* ownedMemory, size_t size) {
	memory = ownedMemory;
	totalMemorySize = size;
	hasAllocatedOwnMemory = false;
}

bool LinearAllocator::Initialize(size_t size) {
	memory = malloc(size);
	totalMemorySize = size;
	hasAllocatedOwnMemory = true;

	return memory != nullptr;
}

void* LinearAllocator::Allocate(size_t size) {
	if (memory == nullptr) {
		// TODO: Assert
		return nullptr;
	}

	size_t usedSizeAfterAllocation = usedSize + size;
	if (usedSizeAfterAllocation > totalMemorySize) {
		// TODO: Assert
		return nullptr;
	}

	void* block = static_cast<char*>(memory) + usedSize;
	usedSize = usedSizeAfterAllocation;

	return block;
}

void LinearAllocator::Destroy() {
	if (hasAllocatedOwnMemory && memory != nullptr) {
		delete memory;
	}
}

void LinearAllocator::Clear() {
	usedSize = 0;
}

void LinearAllocator::ClearAndZero() {
	if (memory != nullptr) {
		memset(memory, 0, totalMemorySize);
	}

	usedSize = 0;
}
