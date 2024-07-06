#include <cstring>

#include <Common/String.hpp>

#include "MemoryAllocator.hpp"

using namespace Grindstone::Memory;

Grindstone::StringRef AllocatorCore::AllocateString(size_t size) {
	char* memory = static_cast<char*>(Allocate(size));

	return Grindstone::StringRef(memory, size);
}

Grindstone::StringRef AllocatorCore::AllocateString(Grindstone::StringRef srcString) {
	size_t srcStringLength = srcString.size() + 1;

	char* memory = static_cast<char*>(Allocate(srcStringLength));
	memcpy(memory, srcString.data(), srcStringLength);

	return Grindstone::StringRef(memory, srcStringLength - 1);
}

void* AllocatorCore::Allocate(size_t size) {
	return allocator.Allocate(size);
}

bool AllocatorCore::Free(void* memPtr) {
	return allocator.Free(memPtr);
}
