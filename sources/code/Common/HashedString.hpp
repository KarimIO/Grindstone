#pragma once

#include <map>

#include "String.hpp"
#include "Hash.hpp"

namespace Grindstone {
	class HashedString {
	public:

		HashedString();
		HashedString(const wchar_t* inStringRef);
		HashedString(const char* inStringRef);
		HashedString(const String& inString);
		HashedString(StringRef inStringRef);

		void Create(const wchar_t* inStringRef);

		String ToString() const;

	protected:

		uint64_t hashedString;
		static std::map<HashValue, String> nameHashMap;

	private:

		std::map<HashValue, String>::iterator ptr;

	};
}
