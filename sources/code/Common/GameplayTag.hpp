#pragma once

#include "HashedString.h"

namespace Grindstone {
	class GameplayTag {
	public:
		GameplayTag();
		GameplayTag(const String& string);
		GameplayTag(StringRef string);
		GameplayTag(HashedString string);

		String ToString() const;
		HashedString ToHashedString() const;
	private:
		HashedString hashedString;

	};
}
