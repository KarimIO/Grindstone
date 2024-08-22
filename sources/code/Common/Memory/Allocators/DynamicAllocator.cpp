#include <memory>
#include <iostream>
#include <sstream>

#include <EngineCore/Logger.hpp>

#include "DynamicAllocator.hpp"

using namespace Grindstone::Memory::Allocators;

constexpr size_t allocationHeaderSize = sizeof(DynamicAllocator::AllocationHeader);
constexpr size_t freeHeaderSize = sizeof(DynamicAllocator::FreeHeader);

static void RemoveFreeList(DynamicAllocator::FreeHeader*& head, DynamicAllocator::FreeHeader* previousNode, DynamicAllocator::FreeHeader* deleteNode) {
	if (previousNode != nullptr) {
		previousNode->nextFreeBlock = deleteNode->nextFreeBlock;
		return;
	}

	// Is the first node
	if (deleteNode->nextFreeBlock == nullptr) {
		// List only has one element
		head = nullptr;
	}
	else {
		// List has more elements
		head = deleteNode->nextFreeBlock;
	}
}

static void InsertFreeList(DynamicAllocator::FreeHeader*& head, DynamicAllocator::FreeHeader*& previousNode, DynamicAllocator::FreeHeader*& newNode) {
	if (previousNode == nullptr) {
		newNode->nextFreeBlock = head;
		head = newNode;
	}
	else {
		if (previousNode->nextFreeBlock == nullptr) {
			// Last node
			previousNode->nextFreeBlock = newNode;
			newNode->nextFreeBlock = nullptr;
		}
		else {
			//
			// Middle node
			newNode->nextFreeBlock = previousNode->nextFreeBlock;
			previousNode->nextFreeBlock = newNode;
		}
	}
}

static size_t CalculatePadding(size_t baseAddress, size_t alignment) {
	size_t multiplier = (baseAddress / alignment) + 1;
	size_t alignedAddress = multiplier * alignment;
	size_t padding = alignedAddress - baseAddress;
	return padding;
}

static size_t CalculatePaddingWithHeader(size_t baseAddress, size_t alignment, size_t headerSize) {
	size_t padding = CalculatePadding(baseAddress, alignment);
	size_t neededSpace = headerSize;

	if (padding < neededSpace) {
		// Find next aligned address
		neededSpace -= padding;

		if (neededSpace % alignment > 0) {
			padding += alignment * (1 + (neededSpace / alignment));
		}
		else {
			padding += alignment * (neededSpace / alignment);
		}
	}

	return padding;
}

void DynamicAllocator::InitializeImpl(void* ownedMemory, size_t size) {;
	if (ownedMemory == nullptr) {
		return;
	}

	startMemory = ownedMemory;
	endMemory = static_cast<void*>(static_cast<char*>(ownedMemory) + size);

	deleterFn = [this](void* ptr) -> void {
		this->Free(ptr);
	};

	memset(startMemory, 0, size);
	firstFreeHeader = reinterpret_cast<FreeHeader*>(startMemory);
	usedSize = 0;
	peakSize = 0;
	totalMemorySize = size;
	hasAllocatedOwnMemory = false;

	firstFreeHeader->blockSize = size;
	firstFreeHeader->nextFreeBlock = nullptr;
}

void DynamicAllocator::Initialize(void* ownedMemory, size_t size) {
	hasAllocatedOwnMemory = false;
	InitializeImpl(ownedMemory, size);
}

bool DynamicAllocator::Initialize(size_t size) {
	void* newMemory = malloc(size);

	if (newMemory == nullptr) {
		return false;
	}

	hasAllocatedOwnMemory = true;
	InitializeImpl(newMemory, size);
	return true;
}

DynamicAllocator::~DynamicAllocator() {
#ifdef _DEBUG
	for (auto& allocation : nameMap) {
		AllocationHeader* header = reinterpret_cast<AllocationHeader*>(static_cast<char*>(allocation.first) - sizeof(AllocationHeader));
		GPRINT_TRACE_V(LogSource::EngineCore, "Unfreed Memory - {} Size({}): {}", allocation.first, header->blockSize, allocation.second);
	}
#endif

	if (startMemory && hasAllocatedOwnMemory) {
		delete startMemory;
	}
}

size_t DynamicAllocator::GetPeakSize() const {
	return peakSize;
}

size_t DynamicAllocator::GetUsedSize() const {
	return usedSize;
}

void* DynamicAllocator::GetMemory() const {
	return startMemory;
}

size_t DynamicAllocator::GetTotalMemorySize() const {
	return totalMemorySize;
}

void DynamicAllocator::FindAvailable(size_t size, size_t alignment, size_t& padding, FreeHeader*& previousFreeHeader, FreeHeader*& freeHeader) const {
	if (searchPolicy == SearchPolicy::FirstSearch) {
		FindAvailableFirst(size, alignment, padding, previousFreeHeader, freeHeader);
	}
	else if (searchPolicy == SearchPolicy::BestSearch) {
		FindAvailableFirst(size, alignment, padding, previousFreeHeader, freeHeader);
	}
	else {
		GPRINT_FATAL(LogSource::EngineCore, "Invalid search policy");
	}
}

void DynamicAllocator::FindAvailableFirst(size_t size, size_t alignment, size_t& padding, FreeHeader*& previousFreeHeader, FreeHeader*& freeHeader) const {
	FreeHeader* it = firstFreeHeader;
	FreeHeader* itPrev = nullptr;

	while (it != nullptr) {
		padding = CalculatePaddingWithHeader((size_t)it, alignment, sizeof(AllocationHeader));
		size_t requiredSpace = size + padding;
		if (it->blockSize >= requiredSpace) {
			break;
		}
		itPrev = it;
		it = it->nextFreeBlock;
	}

	previousFreeHeader = itPrev;
	freeHeader = it;
}

void DynamicAllocator::FindAvailableBest(size_t size, size_t alignment, size_t& padding, FreeHeader*& previousFreeHeader, FreeHeader*& freeHeader) const {
	constexpr size_t smallestDiff = std::numeric_limits<size_t>::max();
	FreeHeader* bestBlock = nullptr;
	FreeHeader* it = firstFreeHeader;
	FreeHeader* itPrev = nullptr;

	while (it != nullptr) {
		padding = CalculatePaddingWithHeader((size_t)it, alignment, sizeof(AllocationHeader));
		size_t requiredSpace = size + padding;
		if (it->blockSize >= requiredSpace && (it->blockSize - requiredSpace < smallestDiff)) {
			bestBlock = it;
		}
		itPrev = it;
		it = it->nextFreeBlock;
	}

	previousFreeHeader = itPrev;
	freeHeader = it;
}

void DynamicAllocator::Coalesce(FreeHeader* previousNode, FreeHeader* freeNode) {
	if (freeNode->nextFreeBlock != nullptr &&
		(size_t)freeNode + freeNode->blockSize == (size_t)freeNode->nextFreeBlock) {
		freeNode->blockSize += freeNode->nextFreeBlock->blockSize;
		RemoveFreeList(firstFreeHeader, freeNode, freeNode->nextFreeBlock);
	}

	if (previousNode != nullptr &&
		(size_t)previousNode + previousNode->blockSize == (size_t)freeNode) {
		previousNode->blockSize += freeNode->blockSize;
		RemoveFreeList(firstFreeHeader, previousNode, freeNode);
	}
}

void* DynamicAllocator::AllocateRaw(size_t size, size_t alignment, const char* debugName) {
	DynamicAllocator::FreeHeader* previousFreeHeader = nullptr;
	DynamicAllocator::FreeHeader* freeHeader = nullptr;
	size_t padding;

	FindAvailable(size, alignment, padding, previousFreeHeader, freeHeader);

	if (freeHeader == nullptr) {
		return nullptr;
	}

	const size_t alignmentPadding = padding - allocationHeaderSize;
	const size_t requiredSize = size + padding;

	const size_t rest = freeHeader->blockSize - requiredSize;

	if (rest > 0) {
		// Split node
		FreeHeader* newFreeNode = (FreeHeader*)((size_t)freeHeader + requiredSize);
		newFreeNode->blockSize = rest;
		InsertFreeList(firstFreeHeader, freeHeader, newFreeNode);
	}

	RemoveFreeList(firstFreeHeader, previousFreeHeader, freeHeader);

	size_t headerAddress = reinterpret_cast<size_t>(freeHeader) + alignmentPadding;
	size_t dataAddress = headerAddress + allocationHeaderSize;

	AllocationHeader* newHeader = reinterpret_cast<AllocationHeader*>(headerAddress);
	newHeader->blockSize = requiredSize;
	newHeader->padding = static_cast<char>(alignmentPadding);

	usedSize += requiredSize;
	peakSize = std::max(peakSize, usedSize);

	void* voidAddr = reinterpret_cast<void*>(dataAddress);

#ifdef _DEBUG
	nameMap[voidAddr] = debugName;
#endif

	return voidAddr;
}

bool DynamicAllocator::Free(void* ptr) {
	size_t currentAddress = reinterpret_cast<size_t>(ptr);
	size_t headerAddress = currentAddress - sizeof(AllocationHeader);
	AllocationHeader* allocationHeader = (AllocationHeader*)headerAddress ;

	FreeHeader* freeNode = reinterpret_cast<FreeHeader*>(headerAddress);
	freeNode->blockSize = allocationHeader->blockSize + allocationHeader->padding;
	freeNode->nextFreeBlock = nullptr;

	FreeHeader* it = firstFreeHeader;
	FreeHeader* itPrev = nullptr;
	while (it != nullptr) {
		if (ptr < it) {
			InsertFreeList(firstFreeHeader, itPrev, freeNode);
			break;
		}
		itPrev = it;
		it = it->nextFreeBlock;
	}

	usedSize -= freeNode->blockSize;

	Coalesce(itPrev, freeNode);

#ifdef _DEBUG
	nameMap.erase(ptr);
#endif


	return true;
}

bool DynamicAllocator::IsEmpty() const {
	// A minimum of one header is always required!
	return usedSize == 0;
}
