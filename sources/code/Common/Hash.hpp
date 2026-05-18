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

		// Similar to boost hash_combine library, to be used with std::hash. Taken from
		// https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
		inline void Combine([[maybe_unused]] std::size_t& seed) {}

		template <typename T, typename... Rest>
		inline void Combine(std::size_t& seed, const T& v, Rest... rest) {
			std::hash<T> hasher;
			seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			Combine(seed, rest...);
		}

		struct HashPair {
			template <class T1, class T2>
			size_t operator()(const std::pair<T1, T2>& p) const {
				size_t hash1 = std::hash<T1>{}(p.first);
				size_t hash2 = std::hash<T2>{}(p.second);

				return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
			}
		};

	};

}
