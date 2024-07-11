#include <memory>
#include <iostream>
#include <sstream>

#include "DynamicAllocator.hpp"

using namespace Grindstone::Memory::Allocators;

// https://www.gingerbill.org/article/2021/11/30/memory-allocation-strategies-005/

constexpr size_t headerSize = sizeof(DynamicAllocator::Header);

std::string BreakBytes(size_t bytes) {
	if (bytes == 0) {
		return "0b";
	}

	size_t gb = bytes >> 30;
	size_t totalMb = bytes >> 20;
	size_t totalKb = bytes >> 10;

	size_t mb = (bytes >> 20) - (gb << 10);
	size_t kb = (bytes >> 10) - (totalMb << 10);
	size_t b = bytes - (totalKb << 10);

	std::stringstream str;

	if (gb > 0) {
		str << gb << "gb ";
	}

	if (mb > 0) {
		str << mb << "mb ";
	}

	if (kb > 0) {
		str << kb << "kb ";
	}

	if (b > 0) {
		str << b << "b ";
	}

	return str.str();
}

void DynamicAllocator::Initialize(void* ownedMemory, size_t size) {
	rootHeader = reinterpret_cast<Header*>(ownedMemory);
	if (rootHeader == nullptr) {
		return;
	}

	memset(rootHeader, 0, size);
	usedSize = sizeof(Header);
	totalMemorySize = size;
	hasAllocatedOwnMemory = false;

	rootHeader->nextHeader = nullptr;
	rootHeader->previousHeader = nullptr;
	rootHeader->isAllocated = false;
}

bool DynamicAllocator::Initialize(size_t size) {
	rootHeader = reinterpret_cast<Header*>(malloc(size));
	if (rootHeader == nullptr) {
		return false;
	}

	memset(rootHeader, 0, size);
	usedSize = sizeof(Header);
	totalMemorySize = size;
	hasAllocatedOwnMemory = true;

	rootHeader->nextHeader = nullptr;
	rootHeader->previousHeader = nullptr;
	rootHeader->isAllocated = false;
	return true;
}

size_t DynamicAllocator::GetUsedSize() const {
	return usedSize;
}

void* DynamicAllocator::GetMemory() const {
	return rootHeader;
}

size_t DynamicAllocator::GetTotalMemorySize() const {
	return totalMemorySize;
}

DynamicAllocator::Header* DynamicAllocator::FindAvailableHeader(size_t size) const {
	size_t minimumSpaceRequired = size + sizeof(Header);
	Header* currentMemoryHeader = rootHeader;
	Header* previousMemoryHeader = nullptr;

	while (true) {
		Header* nextMemoryHeader = currentMemoryHeader->nextHeader;
		char* endOfBlock = reinterpret_cast<char*>(nextMemoryHeader);
		size_t spaceLeft = endOfBlock - reinterpret_cast<char*>(currentMemoryHeader);

		if (!currentMemoryHeader->isAllocated && spaceLeft >= minimumSpaceRequired) {
			return currentMemoryHeader;
		}

		if (currentMemoryHeader->nextHeader == nullptr) {
			break;
		}

		currentMemoryHeader = currentMemoryHeader->nextHeader;
	}

	char* end = reinterpret_cast<char*>(rootHeader) + totalMemorySize;
	size_t spaceLeft = end - reinterpret_cast<char*>(currentMemoryHeader);

	if (!currentMemoryHeader->isAllocated && spaceLeft >= minimumSpaceRequired) {
		return currentMemoryHeader;
	}

	return nullptr;
}

void* DynamicAllocator::Allocate(size_t size) {
	DynamicAllocator::Header* header = FindAvailableHeader(size);

	if (header == nullptr) {
		return nullptr;
	}

	char* blockPointer = reinterpret_cast<char*>(header) + sizeof(Header);

	char* nextHeaderAsChar = blockPointer + size;
	char* tail = reinterpret_cast<char*>(rootHeader) + totalMemorySize;
	if (nextHeaderAsChar + sizeof(Header) <= tail) {
		Header* nextHeader = reinterpret_cast<Header*>(nextHeaderAsChar);
		header->nextHeader = nextHeader;
		nextHeader->nextHeader = nullptr;
		nextHeader->previousHeader = header;

		usedSize += size + headerSize;
	}
	else {
		header->nextHeader = nullptr;
		usedSize += size;
	}

	header->isAllocated = true;

	return blockPointer;
}

DynamicAllocator::Header* DynamicAllocator::GetHeaderOfBlock(void* block) {
	char* blockPtrAsChar = static_cast<char*>(block);
	Header* headerPtr = reinterpret_cast<Header*>(blockPtrAsChar - sizeof(Header));

	return headerPtr;
}

bool DynamicAllocator::Free(void* memPtr, bool shouldClear) {
	if (memPtr == nullptr) {
		return false;
	}

	Header* header = GetHeaderOfBlock(memPtr);

	if (header == nullptr) {
		return false;
	}

	header->isAllocated = false;

	char* deleteStart = reinterpret_cast<char*>(header) + sizeof(Header);
	char* deleteEnd = header->nextHeader == nullptr
		? reinterpret_cast<char*>(rootHeader) + totalMemorySize
		: reinterpret_cast<char*>(header->nextHeader);
	size_t removedActualSize = deleteEnd - deleteStart;

	Header* prevHeader = header;
	if (header->previousHeader != nullptr && !header->previousHeader->isAllocated) {
		prevHeader = header->previousHeader;
	}

	Header* nextHeader = header->nextHeader;
	if (nextHeader != nullptr && !nextHeader->isAllocated) {
		nextHeader = nextHeader->nextHeader;
		removedActualSize += sizeof(Header);
	}

	prevHeader->nextHeader = nextHeader;
	if (nextHeader != nullptr) {
		nextHeader->previousHeader = prevHeader;
		removedActualSize += sizeof(Header);
	}

	if (shouldClear) {
		char* clearStart = reinterpret_cast<char*>(header) + sizeof(Header);
		char* clearEnd = header->nextHeader == nullptr
			? reinterpret_cast<char*>(rootHeader) + totalMemorySize
			: reinterpret_cast<char*>(header->nextHeader);

		size_t sizeToClear = clearEnd - clearStart;

		usedSize -= removedActualSize;

		memset(clearStart, 0, sizeToClear);
	}

	return true;
}

void DynamicAllocator::PrintBlocks() {
	Header* currentHeader = rootHeader;
	while (currentHeader != nullptr) {
		const char* allocStr = currentHeader->isAllocated ? "ALLOCATED" : "DEALLOCATED";

		char* target = currentHeader->nextHeader == nullptr
			? reinterpret_cast<char*>(rootHeader) + totalMemorySize
			: reinterpret_cast<char*>(currentHeader->nextHeader);

		size_t bytes = target - reinterpret_cast<char*>(currentHeader) - sizeof(Header);
		std::cout << "[" << allocStr << "]:" << BreakBytes(bytes) << "\n";
		currentHeader = currentHeader->nextHeader;
	}
}

bool DynamicAllocator::IsEmpty() const {
	// A minimum of one header is always required!
	return usedSize == sizeof(Header);
}

void* ValidatePtr(void* block) {
	using Grindstone::Memory::Allocators::DynamicAllocator;
	if (block == nullptr) {
		return nullptr;
	}

	DynamicAllocator::Header* header = DynamicAllocator::GetHeaderOfBlock(block);
	return header->isAllocated
		? block
		: nullptr;
}

bool IsValid(void* block) {
	using Grindstone::Memory::Allocators::DynamicAllocator;
	if (block == nullptr) {
		return false;
	}

	DynamicAllocator::Header* header = DynamicAllocator::GetHeaderOfBlock(block);
	return header->isAllocated;
}
