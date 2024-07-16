#include <cstring>

#include <Common/String.hpp>

#include "MemoryAllocator.hpp"

using namespace Grindstone::Memory;

bool AllocatorCore::Initialize(size_t sizeInMegs) {
	return allocator.Initialize(sizeInMegs * 1024u * 1024u);
}

Grindstone::StringRef AllocatorCore::AllocateString(size_t size) {
	char* memory = static_cast<char*>(allocator.AllocateRaw(size));

	return Grindstone::StringRef(memory, size);
}

Grindstone::StringRef AllocatorCore::AllocateString(Grindstone::StringRef srcString) {
	size_t srcStringLength = srcString.size() + 1;

	char* memory = static_cast<char*>(allocator.AllocateRaw(srcStringLength));
	memcpy(memory, srcString.data(), srcStringLength);

	return Grindstone::StringRef(memory, srcStringLength - 1);
}

bool AllocatorCore::FreeWithoutDestructor(void* memPtr) {
	return allocator.Free(memPtr);
}

size_t Grindstone::Memory::AllocatorCore::GetUsed() const {
	return allocator.GetUsedSize();
}

size_t Grindstone::Memory::AllocatorCore::GetTotal() const {
	return allocator.GetTotalMemorySize();
}

bool Grindstone::Memory::AllocatorCore::IsEmpty() const {
	return allocator.IsEmpty();
}
