#include <memory>
#include <iostream>
#include <sstream>

#include <EngineCore/Logger.hpp>

#include "DynamicAllocator.hpp"
#include <Assert.hpp>

using namespace Grindstone::Memory::Allocators;

constexpr size_t allocationHeaderSize = sizeof(DynamicAllocator::AllocationHeader);
constexpr size_t freeHeaderSize = sizeof(DynamicAllocator::FreeHeader);

static constexpr size_t minBlockSize = std::max(sizeof(DynamicAllocator::FreeHeader), sizeof(DynamicAllocator::AllocationHeader) + 1);
static constexpr size_t maxMetadataAlignment = std::max(alignof(DynamicAllocator::FreeHeader), alignof(DynamicAllocator::AllocationHeader));

static void FreeListInsert(DynamicAllocator::FreeHeader*& head, DynamicAllocator::FreeHeader* previousNode, DynamicAllocator::FreeHeader* newNode) {
	if (previousNode == nullptr) {
		// Is the first node
		if (head != nullptr) {
			// The list has more elements
			newNode->nextFreeBlock = head;
		}
		else {
			newNode->nextFreeBlock = nullptr;
		}
		head = newNode;
	}
	else {
		if (previousNode->nextFreeBlock == nullptr) {
			// Is the last node
			previousNode->nextFreeBlock = newNode;
			newNode->nextFreeBlock = nullptr;
		}
		else {
			// Is a middle node
			newNode->nextFreeBlock = previousNode->nextFreeBlock;
			previousNode->nextFreeBlock = newNode;
		}
	}
}

static void FreeListRemove(DynamicAllocator::FreeHeader*& head, DynamicAllocator::FreeHeader* previousNode, DynamicAllocator::FreeHeader* deleteNode) {
	if (previousNode == nullptr) {
		if (deleteNode->nextFreeBlock == nullptr) {
			head = nullptr;
		}
		else {
			head = deleteNode->nextFreeBlock;
		}
	}
	else {
		previousNode->nextFreeBlock = deleteNode->nextFreeBlock;
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

static void FindBlockFromFreelistFirst(DynamicAllocator::FreeHeader* head, size_t size, size_t alignment, size_t& padding, DynamicAllocator::FreeHeader*& nodeFound, DynamicAllocator::FreeHeader*& nodeFoundPrevious) {
	nodeFound = head;
	nodeFoundPrevious = nullptr;

	while (nodeFound != nullptr) {
		padding = CalculatePaddingWithHeader(reinterpret_cast<size_t>(nodeFound), alignment, allocationHeaderSize);
		size_t requiredSpace = size + padding;
		if (nodeFound->blockSize >= requiredSpace) {
			break;
		}

		nodeFoundPrevious = nodeFound;
		nodeFound = nodeFound->nextFreeBlock;
	}
}

static void FindBlockFromFreelistBest(DynamicAllocator::FreeHeader* head, size_t size, size_t alignment, size_t& bestPadding, DynamicAllocator::FreeHeader*& bestNodeFound, DynamicAllocator::FreeHeader*& bestNodeFoundPrevious) {
	size_t smallestSuitableBlockSize = std::numeric_limits<size_t>::max();
	size_t currentPadding = 0;
	DynamicAllocator::FreeHeader* node = head;
	DynamicAllocator::FreeHeader* previous = nullptr;
	bestNodeFound = nullptr;
	bestNodeFoundPrevious = nullptr;

	while (node != nullptr) {
		currentPadding = CalculatePaddingWithHeader(reinterpret_cast<size_t>(node), alignment, allocationHeaderSize);
		size_t requiredSpace = size + currentPadding;
		if (node->blockSize == requiredSpace) {
			bestPadding = currentPadding;
			bestNodeFound = node;
			bestNodeFoundPrevious = previous;
			break;
		}
		else if (node->blockSize > requiredSpace && (node->blockSize - requiredSpace) < smallestSuitableBlockSize) {
			smallestSuitableBlockSize = node->blockSize;
			bestPadding = currentPadding;
			bestNodeFound = node;
			bestNodeFoundPrevious = previous;
		}

		previous = node;
		node = node->nextFreeBlock;
	}
}

static void FindBlockFromFreelist(DynamicAllocator::SearchPolicy policy, DynamicAllocator::FreeHeader* head, size_t size, size_t alignment, size_t& bestPadding, DynamicAllocator::FreeHeader*& selectedNode, DynamicAllocator::FreeHeader*& previousNode) {
	if (policy == DynamicAllocator::SearchPolicy::FirstSearch) {
		FindBlockFromFreelistFirst(head, size, alignment, bestPadding, selectedNode, previousNode);
	}
	else if (policy == DynamicAllocator::SearchPolicy::BestSearch) {
		FindBlockFromFreelistBest(head, size, alignment, bestPadding, selectedNode, previousNode);
	}
	else {
		GPRINT_FATAL(Grindstone::LogSource::EngineCore, "Invalid search policy");
	}
}

void DynamicAllocator::InitializeImpl(void* ownedMemory, size_t size) {
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
		free(startMemory);
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

void* DynamicAllocator::AllocateRaw(size_t size, size_t alignment, const char* debugName) {
	DynamicAllocator::FreeHeader* previousNode = nullptr;
	DynamicAllocator::FreeHeader* selectedNode = nullptr;
	size_t padding = 0;
	FindBlockFromFreelist(searchPolicy, firstFreeHeader, size, alignment, padding, selectedNode, previousNode);

	if (selectedNode == nullptr) {
		return nullptr;
	}

	const std::size_t alignmentPadding = padding - allocationHeaderSize;
	std::size_t requiredSize = size + padding;
	requiredSize = (requiredSize + alignof(DynamicAllocator::FreeHeader) - 1) & ~(alignof(DynamicAllocator::FreeHeader) - 1);

	const std::size_t rest = selectedNode->blockSize - requiredSize;

	if (rest > minBlockSize) {
		// We have to split the block into the data block and a free block of size 'rest'
		DynamicAllocator::FreeHeader* newFreeNode = (DynamicAllocator::FreeHeader*)((std::size_t)selectedNode + requiredSize);
		newFreeNode->blockSize = rest;
		FreeListInsert(firstFreeHeader, selectedNode, newFreeNode);

		GS_ASSERT(
			reinterpret_cast<uintptr_t>(newFreeNode) %
			alignof(FreeHeader) == 0
		);
	}

	FreeListRemove(firstFreeHeader, previousNode, selectedNode);

	const std::size_t headerAddress = (std::size_t)selectedNode + alignmentPadding;
	const std::size_t dataAddress = headerAddress + allocationHeaderSize;

	((DynamicAllocator::AllocationHeader*)headerAddress)->blockSize = requiredSize;
	((DynamicAllocator::AllocationHeader*)headerAddress)->padding = alignmentPadding;

	usedSize += requiredSize;
	peakSize = std::max(peakSize, usedSize);

	void* voidAddr = reinterpret_cast<void*>(dataAddress);

#ifdef _DEBUG
	strncpy_s(nameMap[voidAddr], debugName, DEBUG_NAME_SIZE - 1);
#endif

	return voidAddr;
}

static void Coalesce(DynamicAllocator::FreeHeader*& head, DynamicAllocator::FreeHeader* previousNode, DynamicAllocator::FreeHeader* freeNode) {
	if (freeNode->nextFreeBlock != nullptr && (std::size_t)freeNode + freeNode->blockSize == (std::size_t)freeNode->nextFreeBlock) {
		freeNode->blockSize += freeNode->nextFreeBlock->blockSize;
		FreeListRemove(head, freeNode, freeNode->nextFreeBlock);
	}

	if (previousNode != nullptr &&
		(std::size_t)previousNode + previousNode->blockSize == (std::size_t)freeNode) {
		previousNode->blockSize += freeNode->blockSize;
		FreeListRemove(head, previousNode, freeNode);
	}
}

bool DynamicAllocator::Free(void* ptr) {
	size_t currentAddress = reinterpret_cast<size_t>(ptr);
	AllocationHeader* allocationHeader = reinterpret_cast<AllocationHeader*>(currentAddress - allocationHeaderSize);

	FreeHeader* freeNode = (FreeHeader*)(allocationHeader);
	freeNode->blockSize = allocationHeader->blockSize;
	freeNode->nextFreeBlock = nullptr;

	FreeHeader* it = firstFreeHeader;
	FreeHeader* itPrev = nullptr;

	// Find the previous node
	while (it != nullptr) {
		if (ptr < it) {
			FreeListInsert(firstFreeHeader, itPrev, freeNode);
			GS_ASSERT(
				reinterpret_cast<uintptr_t>(freeNode) %
				alignof(FreeHeader) == 0
			);
			break;
		}

		itPrev = it;
		it = it->nextFreeBlock;
	}

	if (it == nullptr) {
		FreeListInsert(firstFreeHeader, itPrev, freeNode);
		GS_ASSERT(
			reinterpret_cast<uintptr_t>(freeNode) %
			alignof(FreeHeader) == 0
		);
	}

	usedSize -= freeNode->blockSize;

	Coalesce(firstFreeHeader, itPrev, freeNode);

	return true;
}

bool DynamicAllocator::IsEmpty() const {
	// A minimum of one header is always required!
	return usedSize == 0;
}
