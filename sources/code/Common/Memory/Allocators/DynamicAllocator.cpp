#include <memory>
#include <iostream>
#include <sstream>

#include <EngineCore/Logger.hpp>

#include "DynamicAllocator.hpp"

using namespace Grindstone::Memory::Allocators;

constexpr size_t allocationHeaderSize = sizeof(DynamicAllocator::AllocationHeader);
constexpr size_t freeHeaderSize = sizeof(DynamicAllocator::FreeHeader);

static constexpr size_t minBlockSize = std::max(sizeof(DynamicAllocator::FreeHeader), sizeof(DynamicAllocator::AllocationHeader) + 1);
static constexpr size_t maxMetadataAlignment = std::max(alignof(DynamicAllocator::FreeHeader), alignof(DynamicAllocator::AllocationHeader));

static void ClearFreeBlock(DynamicAllocator::FreeHeader* node) {
	node->blockSize = 0;
	node->nextFreeBlock = nullptr;
}

template<typename PtrType>
static void* OffsetPointerVoid(PtrType ptr, size_t size) {
	return reinterpret_cast<void*>(reinterpret_cast<char*>(ptr) + size);
}

template<typename PtrType>
static PtrType OffsetPointer(PtrType ptr, size_t size) {
	return reinterpret_cast<PtrType>(reinterpret_cast<char*>(ptr) + size);
}

static void* ExtractLargeBlock(DynamicAllocator::FreeHeader* block, size_t size, size_t& outputBlockSize) {
	if (block->blockSize - size < minBlockSize) {
		outputBlockSize = block->blockSize;
		return block;
	}
	else {
		outputBlockSize = size;
		block->blockSize -= size;
		return OffsetPointerVoid(block, block->blockSize);
	}
}

static void* AllocateBlockFromFreelistFirst(DynamicAllocator::FreeHeader*& head, size_t size, size_t& outputBlockSize) {
	DynamicAllocator::FreeHeader* node = head;
	DynamicAllocator::FreeHeader* previous = nullptr;

	while (node) {
		if (node->blockSize == size) {
			outputBlockSize = size;
			if (previous == nullptr) {
				head = head->nextFreeBlock;
				return node;
			}
			else {
				previous->nextFreeBlock = node->nextFreeBlock;
				return node;
			}
		}
		else if (node->blockSize > size) {
			return ExtractLargeBlock(node, size, outputBlockSize);
		}

		previous = node;
		node = node->nextFreeBlock;
	}

	return nullptr;
}

static void* AllocateBlockFromFreelistBest(DynamicAllocator::FreeHeader*& head, size_t size, size_t& outputBlockSize) {
	size_t smallestSuitableBlockSize = std::numeric_limits<size_t>::max();
	DynamicAllocator::FreeHeader* bestBlock = nullptr;
	DynamicAllocator::FreeHeader* node = head;
	DynamicAllocator::FreeHeader* previous = nullptr;

	while (node) {
		if (node->blockSize == size) {
			outputBlockSize = size;
			if (previous == nullptr) {
				head = head->nextFreeBlock;
				return node;
			}
			else {
				previous->nextFreeBlock = node->nextFreeBlock;
				return node;
			}
		}
		else if (node->blockSize > size && node->blockSize < smallestSuitableBlockSize) {
			smallestSuitableBlockSize = node->blockSize;
			bestBlock = node;
		}

		previous = node;
		node = node->nextFreeBlock;
	}

	if (bestBlock) {
		return ExtractLargeBlock(bestBlock, size, outputBlockSize);
	}

	return nullptr;
}

static void* AllocateBlockFromFreelist(DynamicAllocator::SearchPolicy policy, DynamicAllocator::FreeHeader*& head, size_t size, size_t& outputBlockSize) {
	if (policy == DynamicAllocator::SearchPolicy::BestSearch) {
		return AllocateBlockFromFreelistFirst(head, size, outputBlockSize);
	}
	else if (policy == DynamicAllocator::SearchPolicy::FirstSearch) {
		return AllocateBlockFromFreelistBest(head, size, outputBlockSize);
	}
	else {
		GPRINT_FATAL(Grindstone::LogSource::EngineCore, "Invalid search policy");
		return nullptr;
	}
}

static bool FreeBlockFromFreelist(DynamicAllocator::FreeHeader*& head, void* itemToRemove, size_t size) {
	if (head == nullptr) {
		// Do something
		return false;
	}

	DynamicAllocator::FreeHeader* node = head;
	DynamicAllocator::FreeHeader* previous = nullptr;

	while (node) {
		// The first case is if we find the block directly before the one we want to remove - the normal case.
		if ((node + node->blockSize) == itemToRemove) {
			node->blockSize += size;

			// Coalesce this node and the following one in the case that they can be merged.
			DynamicAllocator::FreeHeader* nodeAfterFreeNode = node->nextFreeBlock;
			if (nodeAfterFreeNode != nullptr && nodeAfterFreeNode == OffsetPointer(node, node->blockSize)) {
				node->blockSize += nodeAfterFreeNode->blockSize;
				node->nextFreeBlock = nodeAfterFreeNode->nextFreeBlock;
				ClearFreeBlock(nodeAfterFreeNode);
			}

			return true;
		}
		// The second case is if we find the node we want to remove - this means that we are clearing an item twice.
		else if (node == itemToRemove) {
			// Trying to clear an already cleared node.
			return false;
		}
		// We want to free an allocated block that is after another allocated block
		else if (node > itemToRemove) {
			// Iterated beyond the space to be freed. Need a new node.
			DynamicAllocator::FreeHeader* newNode = reinterpret_cast<DynamicAllocator::FreeHeader*>(itemToRemove);
			newNode->blockSize = size;

			// If there is a previous node, the new node should be inserted between this and it.
			if (previous) {
				previous->nextFreeBlock = newNode;
				newNode->nextFreeBlock = node;
			}
			else {
				// Otherwise, the new node becomes the head.
				newNode->nextFreeBlock = node;
				head = newNode;
			}

			// Coalesce with the next node
			DynamicAllocator::FreeHeader* nodeAfterFreeNode = node->nextFreeBlock;
			if (nodeAfterFreeNode && OffsetPointer(newNode, newNode->blockSize) == nodeAfterFreeNode) {
				newNode->blockSize += nodeAfterFreeNode->blockSize;
				newNode->nextFreeBlock = nodeAfterFreeNode->nextFreeBlock;
				ClearFreeBlock(nodeAfterFreeNode);
			}

			// Coalesce with the previous node
			if (previous && OffsetPointer(previous, previous->blockSize) == newNode) {
				previous->blockSize += newNode->blockSize;
				previous->nextFreeBlock = newNode->nextFreeBlock;
				ClearFreeBlock(newNode);
			}

			return true;
		}
		// The last free block is still before the block we want - so it is likely surrounded by non free blocks.
		else if (node->nextFreeBlock == nullptr && OffsetPointer(node, node->blockSize) < itemToRemove) {
			DynamicAllocator::FreeHeader* newNode = reinterpret_cast<DynamicAllocator::FreeHeader*>(itemToRemove);
			newNode->blockSize = size;
			newNode->nextFreeBlock = 0;
			node->nextFreeBlock = newNode;

			return true;
		}

		previous = node;
		node = node->nextFreeBlock;
	}

	return false;
}

static void* GetAligned(void* ptr, size_t alignment) {
	return reinterpret_cast<void*>(reinterpret_cast<size_t>(ptr) + (alignment - 1) & ~(alignment - 1));
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

void* DynamicAllocator::AllocateRaw(size_t size, size_t alignment, const char* debugName) {
	DynamicAllocator::FreeHeader* previousFreeHeader = nullptr;
	DynamicAllocator::FreeHeader* freeHeader = nullptr;

	size_t requiredSize = alignment + allocationHeaderSize + size;
	size_t outputBlockSize = 0;
	void* allocatedBlock = AllocateBlockFromFreelist(searchPolicy, firstFreeHeader, requiredSize, outputBlockSize);

	if (allocatedBlock == nullptr) {
		return nullptr;
	}

	void* alignedBlock = GetAligned(reinterpret_cast<char*>(allocatedBlock) + allocationHeaderSize, alignment);
	AllocationHeader* newHeader = reinterpret_cast<AllocationHeader*>(reinterpret_cast<char*>(alignedBlock) - allocationHeaderSize);
	newHeader->blockSize = outputBlockSize;
	newHeader->padding = static_cast<uint8_t>(reinterpret_cast<char*>(newHeader) - reinterpret_cast<char*>(allocatedBlock));

	usedSize += outputBlockSize;
	peakSize = std::max(peakSize, usedSize);

	void* voidAddr = reinterpret_cast<void*>(reinterpret_cast<char*>(newHeader) + sizeof(AllocationHeader));

#ifdef _DEBUG
	nameMap[voidAddr] = debugName;
#endif

	return voidAddr;
}

bool DynamicAllocator::Free(void* ptr) {
	size_t currentAddress = reinterpret_cast<size_t>(ptr);
	AllocationHeader* allocationHeader = reinterpret_cast<AllocationHeader*>(currentAddress - sizeof(AllocationHeader));
	void* blockStart = reinterpret_cast<char*>(allocationHeader) - allocationHeader->padding;
	size_t requiredSize = allocationHeader->blockSize;
	if (FreeBlockFromFreelist(firstFreeHeader, blockStart, requiredSize)) {
		usedSize -= requiredSize;
#ifdef _DEBUG
		nameMap.erase(ptr);
#endif
	}

	return true;
}

bool DynamicAllocator::IsEmpty() const {
	// A minimum of one header is always required!
	return usedSize == 0;
}
