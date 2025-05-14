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
		uint64_t GetHash() const;

		operator bool() const noexcept;
		bool operator==(const Grindstone::HashedString& other) const noexcept;

		const String& ToString() const;

	protected:
		uint64_t hash;
		static std::map<HashValue, String> nameHashMap;

	};
}

namespace std
{
	template <>
	struct hash<Grindstone::HashedString> {
		std::size_t operator()(const Grindstone::HashedString& c) const {
			uint64_t hash = c.GetHash();
			return static_cast<std::size_t>(hash);
		}
	};
}
