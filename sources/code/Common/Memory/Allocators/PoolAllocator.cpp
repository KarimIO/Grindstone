#include "../../Assert.hpp"
#include "PoolAllocator.hpp"

using namespace Grindstone::Memory::Allocators;

// ============================================
// Base Allocator Methods
// ============================================

void BasePoolAllocator::Clear() {
	usedChunkCount = 0;
}

void BasePoolAllocator::ClearAndZero() {
	if (memory != nullptr) {
		memset(memory, 0, totalMemorySize);
	}

	usedChunkCount = 0;
}

void BasePoolAllocator::Destroy() {
	if (hasAllocatedOwnMemory && memory != nullptr) {
		delete memory;
	}
}

void* BasePoolAllocator::AllocateImpl() {
#ifdef _DEBUG
	if (memory == nullptr) {
		GS_BREAK_WITH_MESSAGE("No memory buffer allocated.");
		return nullptr;
	}
#endif

	if (headFreePtr == nullptr || usedChunkCount >= totalChunkCount) {
		GS_BREAK_WITH_MESSAGE("Cannot allocate memory, pool is full.");
		return nullptr;
	}

	void* returnedChunk = headFreePtr;
	headFreePtr = headFreePtr->next;
	++usedChunkCount;

	return returnedChunk;
}

void BasePoolAllocator::DeallocateImpl(void* ptr) {
	--usedChunkCount;

	reinterpret_cast<FreeLink*>(ptr)->next = headFreePtr;
	headFreePtr = reinterpret_cast<FreeLink*>(ptr);

#ifdef _DEBUG
	memset(reinterpret_cast<char*>(ptr) + sizeof(FreeLink), 0xcd, chunkSize - sizeof(FreeLink));
#endif
}

void BasePoolAllocator::DeallocateImpl(size_t index) {
	size_t chunkOffset = index * chunkSize;
	DeallocateImpl(reinterpret_cast<char*>(memory) + chunkOffset);
}

void BasePoolAllocator::SetupLinkedList() {
#ifdef _DEBUG
	memset(memory, 0xcd, totalMemorySize);
#endif

	FreeLink* chunk = static_cast<FreeLink*>(memory);
	headFreePtr = chunk;

	for (size_t i = 0; i < totalChunkCount - 1; ++i) {
		chunk->next = reinterpret_cast<FreeLink*>(reinterpret_cast<char*>(chunk) + chunkSize);
		chunk = chunk->next;
	}

	chunk->next = nullptr;
}


bool BasePoolAllocator::IsEmpty() const {
	return usedChunkCount == 0;
}

size_t BasePoolAllocator::GetUsedCount() const {
	return usedChunkCount;
}

// ============================================
// Generic Allocator Methods
// ============================================

GenericPoolAllocator::~GenericPoolAllocator() {
	Destroy();
}

void GenericPoolAllocator::Initialize(void* ownedMemory, size_t totalSize, size_t sizePerChunk) {
	memory = ownedMemory;
	totalMemorySize = totalSize;
	chunkSize = sizePerChunk;
	usedChunkCount = 0;
	totalChunkCount = totalSize / sizePerChunk;
	hasAllocatedOwnMemory = false;

	SetupLinkedList();
}

bool GenericPoolAllocator::Initialize(size_t sizePerChunk, size_t maxChunkCount) {
	totalMemorySize = sizePerChunk * maxChunkCount;
	totalChunkCount = maxChunkCount;
	chunkSize = sizePerChunk;
	usedChunkCount = 0;
	memory = malloc(totalMemorySize);
	hasAllocatedOwnMemory = true;

	SetupLinkedList();

	return memory != nullptr;
}

void* GenericPoolAllocator::Allocate() {
	return AllocateImpl();
}
