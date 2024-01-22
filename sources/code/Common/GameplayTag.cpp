#include "GameplayTag.h"

namespace Grindstone {
	GameplayTag::GameplayTag() : hashedString() {}

	GameplayTag::GameplayTag(const String& string) : hashedString(string) {}

	GameplayTag::GameplayTag(StringRef string) : hashedString(string) {}

	GameplayTag::GameplayTag(HashedString string) : hashedString(string) {}

	String GameplayTag::ToString() const {
		return hashedString.ToString();
	}

	HashedString GameplayTag::ToHashedString() const {
		return hashedString;
	}
}
