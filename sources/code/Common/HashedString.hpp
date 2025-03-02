#pragma once

#include <map>

#include "String.hpp"
#include "Hash.hpp"

namespace Grindstone {
	class HashedString {
	public:

		HashedString();
		HashedString(const char* inStringRef);
		HashedString(const String& inString);
		HashedString(StringRef inStringRef);

		void Create(const char* inStringRef);

		operator bool() const noexcept;
		bool operator==(Grindstone::HashedString& other) const noexcept;

		const String& ToString() const;

	protected:
		uint64_t hash;
		static std::map<HashValue, String> nameHashMap;

	};
}
