#include <stdlib.h>

#include "HashedString.hpp"

Grindstone::HashedString::HashedString() : hash(0) {}

Grindstone::HashedString::HashedString(const char* inStringRef) {
	Create(inStringRef);
}

Grindstone::HashedString::HashedString(const String& inString) : HashedString(inString.data()) {}
Grindstone::HashedString::HashedString(StringRef inStringRef) : HashedString(inStringRef.data()) {}

Grindstone::HashedString::operator bool() const noexcept {
	return hash != 0;
}

bool Grindstone::HashedString::operator==(const Grindstone::HashedString& other) const noexcept {
	return hash == other.hash;
}

void Grindstone::HashedString::Create(const char* inStringRef) {
	hash = Hash::MurmurOAAT64(inStringRef);
}

uint64_t Grindstone::HashedString::GetHash() const {
	return hash;
}

const Grindstone::String& Grindstone::HashedString::ToString() const {
	return "";
}
