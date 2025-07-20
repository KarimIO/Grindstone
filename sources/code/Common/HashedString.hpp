#pragma once

#include <map>

#include "String.hpp"
#include "Hash.hpp"

namespace Grindstone {
	class HashedString {
	public:
		using HashMap = std::map<Grindstone::HashValue, Grindstone::String>;
		static void CreateHashMap();
		static HashMap* GetHashedStringMap();
		static void SetHashMap(HashMap* hashMap);
		HashedString();
		explicit HashedString(const char* inStringRef);
		explicit HashedString(const String& inString);
		explicit HashedString(StringRef inStringRef);

		void Create(StringRef inStringRef);
		uint64_t GetHash() const;

		operator bool() const noexcept;
		bool operator==(const Grindstone::HashedString& other) const noexcept;

		bool operator>(const HashedString& other) const {
			return hash > other.hash;
		}

		bool operator<(const HashedString& other) const {
			return hash < other.hash;
		}

		bool operator>=(const HashedString& other) const {
			return hash >= other.hash;
		}

		bool operator<=(const HashedString& other) const {
			return hash <= other.hash;
		}

		operator std::size_t() const {
			return static_cast<std::size_t>(hash);
		}


		const String& ToString() const;

	protected:
		Grindstone::HashValue hash;
	};

	class ConstHashedString {
	public:
		constexpr ConstHashedString(const char* str)
			: hash(Hash::MurmurOAAT64(str)), literal(str) {
		}

		bool operator==(const Grindstone::HashedString& other) const noexcept {
			return hash == other.GetHash();
		}

		bool operator==(const Grindstone::ConstHashedString& other) const noexcept {
			return hash == other.GetHash();
		}

		friend bool operator==(const Grindstone::HashedString& a, const Grindstone::ConstHashedString& b) noexcept {
			return a.GetHash() == b.GetHash();
		}

		uint64_t GetHash() const noexcept { return hash; }
		operator uint64_t() const noexcept { return hash; }
		HashedString GetHashedString() const noexcept { return HashedString(literal); }
		operator HashedString() const noexcept { return HashedString(literal); }

		const char* ToString() const {
			auto map = Grindstone::HashedString::GetHashedStringMap();
			if (map != nullptr) {
				auto it = map->find(hash);
				if (it == map->end()) {
					map->emplace(hash, Grindstone::String(literal));
				}
			}

			return literal;
		}

	private:
		uint64_t hash;
		const char* literal;
	};
}

namespace std {
	template <>
	struct hash<Grindstone::HashedString> {
		std::size_t operator()(const Grindstone::HashedString& c) const {
			uint64_t hash = c.GetHash();
			return static_cast<std::size_t>(hash);
		}
	};
}
