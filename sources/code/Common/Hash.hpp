#pragma once

#include "IntTypes.h"

namespace Grindstone {
	using HashValue = uint64;

	namespace Hash {
		constexpr HashValue MurmurOAAT64(const wchar_t* key);
	};

	constexpr HashValue Hash::MurmurOAAT64(const wchar_t* key) {
		HashValue h(525201411107845655ull);
		for (; *key; ++key) {
			h ^= *key;
			h *= 0x5bd1e9955bd1e995;
			h ^= h >> 47;
		}
		return h;
	}
}
