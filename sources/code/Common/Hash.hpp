#pragma once

#include "IntTypes.hpp"

namespace Grindstone {
	using HashValue = Uint64;

	namespace Hash {
		constexpr HashValue MurmurOAAT64(const char* key) {
			HashValue h(525201411107845655ull);
			for (; *key; ++key) {
				h ^= *key;
				h *= 0x5bd1e9955bd1e995;
				h ^= h >> 47;
			}
			return h;
		}

		constexpr HashValue MurmurOAAT64(const char* key, size_t length) {
			HashValue h(525201411107845655ull);
			for (size_t i = 0; i < length; ++i) {
				h ^= key[i];
				h *= 0x5bd1e9955bd1e995;
				h ^= h >> 47;
			}
			return h;
		}

		constexpr HashValue MurmurOAAT64(HashValue h, const char* key) {
			for (; *key; ++key) {
				h ^= *key;
				h *= 0x5bd1e9955bd1e995;
				h ^= h >> 47;
			}
			return h;
		}

		constexpr HashValue CombineMurmurOAAT64(HashValue h, const char* key, size_t length) {
			for (size_t i = 0; i < length; ++i) {
				h ^= key[i];
				h *= 0x5bd1e9955bd1e995;
				h ^= h >> 47;
			}
			return h;
		}
	};

}
