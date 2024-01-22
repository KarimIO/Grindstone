#include <stdlib.h>

#include "HashedString.h"

namespace Grindstone {
	// TODO: Consider removing map value.
	std::map<HashValue, String> HashedString::nameHashMap;

	HashedString::HashedString() : hashedString(0) {}

	HashedString::HashedString(const wchar_t* inStringRef) {
		Create(inStringRef);
	}

	HashedString::HashedString(const char* inStringRef) {
		const size_t size = strlen(inStringRef) + 1;
		wchar_t* wText = new wchar_t[size];

		size_t outSize;
		mbstowcs_s(&outSize, wText, size, inStringRef, size - 1);

		Create(wText);
	}

	HashedString::HashedString(const String& inString) : HashedString(inString.data()) {}
	HashedString::HashedString(StringRef inStringRef) : HashedString(inStringRef.data()) {}

	void HashedString::Create(const wchar_t* inStringRef) {
		hashedString = Hash::MurmurOAAT64(inStringRef);
		nameHashMap[hashedString] = inStringRef;
		ptr = nameHashMap.find(hashedString);
	}

	String HashedString::ToString() const {
		if (hashedString == 0) {
			return GS_TEXT("");
		}

		std::map<HashValue, String>::iterator value = nameHashMap.find(hashedString);

		if (value == nameHashMap.end()) {
			return GS_TEXT("");
		}

		return value->second;
	}
}
