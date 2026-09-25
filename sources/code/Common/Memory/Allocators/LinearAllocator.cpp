#include <cstring>
#include <malloc.h>
#include <new>

#include <Common/Assert.hpp>
#include "LinearAllocator.hpp"

static size_t CalculatePadding(size_t baseAddress, size_t alignment) {
	const size_t remainder = baseAddress % alignment;
	return remainder == 0 ? 0 : alignment - remainder;
}

using namespace Grindstone::Memory::Allocators;

LinearAllocator::~LinearAllocator() {
	Destroy();
}

void LinearAllocator::Initialize(void* ownedMemory, size_t size) {
	memory = ownedMemory;
	usedSize = 0;
	totalMemorySize = size;
	hasAllocatedOwnMemory = false;
}

bool LinearAllocator::Initialize(size_t size) {
	memory = std::malloc(size);
	usedSize = 0;
	totalMemorySize = size;
	hasAllocatedOwnMemory = true;

	return memory != nullptr;
}

void* LinearAllocator::AllocateRaw(size_t size, size_t alignment) {
#ifdef _DEBUG
	if (memory == nullptr) {
		GS_BREAK_WITH_MESSAGE("No memory buffer allocated.");
		return nullptr;
	}
#endif

	size_t padding =
		CalculatePadding(reinterpret_cast<size_t>(memory), alignment);
	size_t usedSizeAfterAllocation = usedSize + padding + size;
	if (usedSizeAfterAllocation > totalMemorySize) {
		GS_BREAK_WITH_MESSAGE("Cannot allocate memory.");
		return nullptr;
	}

	void* block = static_cast<char*>(memory) + padding + usedSize;
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

size_t LinearAllocator::GetUsedSize() const {
	return usedSize;
}
