#include <cstring>

#include <Common/String.hpp>

#include "MemoryAllocator.hpp"

using namespace Grindstone::Memory;

static Grindstone::Memory::AllocatorCore::AllocatorState* allocatorState = nullptr;

Grindstone::Memory::AllocatorCore::AllocatorState* Grindstone::Memory::AllocatorCore::GetAllocatorState() {
	return allocatorState;
}

void Grindstone::Memory::AllocatorCore::SetAllocatorState(Grindstone::Memory::AllocatorCore::AllocatorState* newAllocatorState) {
	allocatorState = newAllocatorState;
}

void* Grindstone::Memory::AllocatorCore::AllocateRaw(size_t size, size_t alignment, const char* debugName) {
	return GetAllocatorState()->dynamicAllocator.AllocateRaw(size, alignment, debugName);
}

bool AllocatorCore::Initialize(size_t sizeInMegs) {
	allocatorState = reinterpret_cast<Grindstone::Memory::AllocatorCore::AllocatorState*>(malloc(sizeof(AllocatorState)));
	if (allocatorState == nullptr) {
		return false;
	}

	return allocatorState->dynamicAllocator.Initialize(sizeInMegs * 1024u * 1024u);
}

void Grindstone::Memory::AllocatorCore::CloseAllocator() {
	delete allocatorState;
}

Grindstone::StringRef AllocatorCore::AllocateString(size_t size) {
	char* memory = static_cast<char*>(allocatorState->dynamicAllocator.AllocateRaw(size, alignof(std::string), "String"));

	return Grindstone::StringRef(memory, size);
}

Grindstone::StringRef AllocatorCore::AllocateString(Grindstone::StringRef srcString) {
	size_t srcStringLength = srcString.size() + 1;

	char* memory = static_cast<char*>(allocatorState->dynamicAllocator.AllocateRaw(srcStringLength, alignof(std::string), "String"));
	memcpy(memory, srcString.data(), srcStringLength);

	return Grindstone::StringRef(memory, srcStringLength - 1);
}

bool AllocatorCore::FreeWithoutDestructor(void* memPtr) {
	return allocatorState->dynamicAllocator.Free(memPtr);
}

size_t Grindstone::Memory::AllocatorCore::GetPeak() {
	return allocatorState->dynamicAllocator.GetPeakSize();
}

size_t Grindstone::Memory::AllocatorCore::GetUsed() {
	return allocatorState->dynamicAllocator.GetUsedSize();
}

size_t Grindstone::Memory::AllocatorCore::GetTotal() {
	return allocatorState->dynamicAllocator.GetTotalMemorySize();
}

bool Grindstone::Memory::AllocatorCore::IsEmpty() {
	return allocatorState->dynamicAllocator.IsEmpty();
}
