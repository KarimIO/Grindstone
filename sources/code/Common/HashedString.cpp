#include <stdlib.h>
#include <map>

#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "HashedString.hpp"
#include "String.hpp"

Grindstone::HashedString::HashMap* nameHashMap = nullptr;

void Grindstone::HashedString::CreateHashMap() {
	nameHashMap = Grindstone::Memory::AllocatorCore::Allocate<HashMap>();
}

Grindstone::HashedString::HashMap* Grindstone::HashedString::GetHashedStringMap() {
	return nameHashMap;
}

void Grindstone::HashedString::SetHashMap(Grindstone::HashedString::HashMap* hashMap) {
	nameHashMap = hashMap;
}

Grindstone::HashedString::HashedString() : hash(0) {}

Grindstone::HashedString::HashedString(const char* inStringRef) {
	Create(inStringRef);
}

Grindstone::HashedString::HashedString(const String& inString) {
	Create(inString);
}

Grindstone::HashedString::HashedString(StringRef inStringRef) {
	Create(inStringRef);
}

Grindstone::HashedString::operator bool() const noexcept {
	return hash != 0;
}

bool Grindstone::HashedString::operator==(const Grindstone::HashedString& other) const noexcept {
	return hash == other.hash;
}

void Grindstone::HashedString::Create(StringRef inStringRef) {
	hash = Hash::MurmurOAAT64(inStringRef.data(), inStringRef.size());
	nameHashMap->emplace(hash, Grindstone::String(inStringRef));
}

uint64_t Grindstone::HashedString::GetHash() const {
	return hash;
}

const Grindstone::String& Grindstone::HashedString::ToString() const {
	static const Grindstone::String emptyString = "[ EMPTY ]";
	static const Grindstone::String invalidString = "[ INVALID ]";

	if (hash == 0) {
		return emptyString;
	}

	std::map<HashValue, String>::iterator value = nameHashMap->find(hash);

	if (value == nameHashMap->end()) {
		return invalidString;
	}

	return value->second;
}
