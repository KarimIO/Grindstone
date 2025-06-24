#pragma once

#include <limits.h>

#include "../EnumTraits.hpp"
#include "../Assert.hpp"

namespace Grindstone::Containers {
	static uint32_t Popcount(uint32_t n) {
		// https://nimrod.blog/posts/algorithms-behind-popcount/
		n = (n & 0x55555555) + ((n >> 1) & 0x55555555);
		n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
		n = (n & 0x0F0F0F0F) + ((n >> 4) & 0x0F0F0F0F);
		n = (n & 0x00FF00FF) + ((n >> 8) & 0x00FF00FF);
		n = (n & 0x0000FFFF) + ((n >> 16) & 0x0000FFFF);
		return n;
	}

	template<uint32_t bitCount, typename WordType = uint32_t>
	class Bitset {
	public:
		static constexpr uint32_t bitsPerWord = (CHAR_BIT * sizeof(WordType));
		static constexpr uint32_t wordCount = ((bitCount + bitsPerWord - 1) / bitsPerWord);
		static constexpr uint32_t totalBitCount = wordCount * bitsPerWord;
		static constexpr uint32_t lastWordbitCount = bitCount % bitsPerWord;
		static constexpr uint32_t lastWordMask = (lastWordbitCount == 0)
			? ~0
			: ((1 << lastWordbitCount) - 1);

		Bitset() {
			for (size_t i = 0; i < wordCount; ++i) {
				contents[i] = 0;
			}
		}

		Bitset(const Bitset& other) {
			for (size_t i = 0; i < wordCount; ++i) {
				contents[i] = other.contents[i];
			}
		}

		Bitset(Bitset&& other) noexcept {
			for (size_t i = 0; i < wordCount; ++i) {
				contents[i] = other.contents[i];
			}
		}

		Bitset& operator=(const Bitset& other) {
			for (size_t i = 0; i < wordCount; ++i) {
				contents[i] = other.contents[i];
			}
			return *this;
		}

		Bitset& operator=(Bitset&& other) {
			for (size_t i = 0; i < wordCount; ++i) {
				contents[i] = other.contents[i];
			}
			return *this;
		}

		void Set() {
			for (WordType& w : contents) {
				w = WordType(~0);
			}

			// Clear unused bits of last mask, which will now be 1.
			contents[wordCount - 1] &= lastWordMask;
		}

		void Set(uint32_t bitIndex, bool value = true) {
			size_t wordIndex = bitIndex / bitsPerWord;
			size_t bitInWord = bitIndex % bitsPerWord;

			if (value) {
				contents[wordIndex] |= (1 << bitInWord);
			}
			else {
				contents[wordIndex] &= ~(1 << bitInWord);
			}
		}

		void Unset() {
			for (WordType& w : contents) {
				w = 0;
			}
		}

		void Unset(uint32_t index) {
			contents[index / bitsPerWord] &= ~(1 << (index % bitsPerWord));
		}

		void Flip() {
			for (WordType& w : contents) {
				w = ~w;
			}

			// Clear unused bits of last mask, which will now be 1.
			contents[wordCount - 1] &= lastWordMask;
		}

		void Flip(uint32_t index) {
			contents[index / bitsPerWord] ^= (1 << (index % bitsPerWord));
		}

		bool Test(uint32_t index) const {
			return (contents[index / bitsPerWord] >> (index % bitsPerWord)) & 1;
		}

		WordType GetWord(uint32_t index) const {
			return contents[index];
		}

		bool All() const {
			for (size_t i = 0; i < wordCount - 1; ++i) {
				if (contents[i] != WordType(~0)) {
					return true;
				}
			}

			return false;
		}

		bool Any() const {
			for (WordType w : contents) {
				if (w != 0) {
					return true;
				}
			}

			return false;
		}

		bool None() const {
			for (WordType w : contents) {
				if (w != 0) {
					return false;
				}
			}

			return true;
		}

		constexpr uint32_t GetTrueBitCount() const {
			uint32_t total = 0;
			for (uint32_t w : contents) {
				total += Popcount(w);
			}

			return total;
		}

		constexpr uint32_t GetFalseBitCount() const {
			return bitCount - GetTrueBitCount();
		}

		static constexpr uint32_t GetBitCount() {
			return bitCount;
		}

		static constexpr uint32_t GetBitCapacity() {
			return totalBitCount;
		}

		static constexpr uint32_t GetWordCount() {
			return wordCount;
		}

		Bitset operator&=(const Bitset& other) const {
			for (size_t i = 0; i < wordCount; ++i) {
				contents[i] &= other.contents[i];
			}

			return *this;
		}

		Bitset operator|=(const Bitset& other) const {
			for (size_t i = 0; i < wordCount; ++i) {
				contents[i] |= other.contents[i];
			}

			return *this;
		}

		Bitset operator^=(const Bitset& other) const {
			for (size_t i = 0; i < wordCount; ++i) {
				contents[i] ^= other.contents[i];
			}

			return *this;
		}

		Bitset operator&(const Bitset& other) const {
			Bitset result;
			for (size_t i = 0; i < wordCount; ++i) {
				result.contents[i] = contents[i] & other.contents[i];
			}

			return result;
		}

		Bitset operator|(const Bitset& other) const {
			Bitset result;
			for (size_t i = 0; i < wordCount; ++i) {
				result.contents[i] = contents[i] | other.contents[i];
			}

			return result;
		}

		Bitset operator^(const Bitset& other) const {
			Bitset result;
			for (size_t i = 0; i < wordCount; ++i) {
				result.contents[i] = contents[i] ^ other.contents[i];
			}

			return result;
		}

		friend Bitset operator~(const Bitset& obj) {
			Bitset result = obj;
			result.Flip();
			return result;
		}

		bool operator[](uint32_t index) const {
			return (contents[index / bitsPerWord] >> index % bitsPerWord) & 1;
		}

		bool operator==(const Bitset& other) const {
			for (size_t i = 0; i < wordCount; ++i) {
				if (contents[i] != other.contents[i]) {
					return false;
				}
			}

			return true;
		}

		bool operator!=(const Bitset& other) const {
			return *this != other;
		}

	protected:
		WordType contents[wordCount];
	};

	// NOTE: Requires Enum to have an entry called Count with the total number of options.
	template <typename Enum>
	class BitsetEnum : public Bitset<static_cast<std::underlying_type_t<Enum>>(Enum::Count), std::underlying_type_t<Enum>> {
	public:
		using UnderlyingType = std::underlying_type_t<Enum>;
		using Traits = EnumTraits<Enum>;
		static_assert(std::is_enum_v<Enum>, "BitsetFlags requires an enum type");

		static constexpr uint32_t enumBitCount = Traits::size;
		using Base = Bitset<static_cast<UnderlyingType>(Enum::Count), UnderlyingType>;
		using Base::Base;

		using Base::wordCount;
		using Base::bitsPerWord;
		using Base::totalBitCount;
		using Base::lastWordbitCount;
		using Base::lastWordMask;
		using Base::Set;
		using Base::Unset;
		using Base::Flip;
		using Base::Test;
		using Base::GetWord;
		using Base::All;
		using Base::Any;
		using Base::None;
		using Base::GetTrueBitCount;
		using Base::GetFalseBitCount;
		using Base::GetBitCount;
		using Base::GetBitCapacity;
		using Base::GetWordCount;
		using Base::operator&=;
		using Base::operator|=;
		using Base::operator^=;
		using Base::operator&;
		using Base::operator|;
		using Base::operator^;
		using Base::operator[];
		using Base::operator==;
		using Base::operator!=;

		void Set(Enum e, bool value = true) {
			Base::Set(static_cast<UnderlyingType>(e), value);
		}

		void Unset(Enum e) {
			Base::Unset(static_cast<UnderlyingType>(e));
		}

		void Flip(Enum e) {
			Base::Flip(static_cast<UnderlyingType>(e));
		}

		bool Test(Enum e) const {
			return Base::Test(static_cast<UnderlyingType>(e));
		}

		bool operator[](Enum e) const {
			return Test(static_cast<UnderlyingType>(e));
		}

		static constexpr const char* GetEntryName(uint32_t index) {
			return Traits::names[index];
		}

		static constexpr const char* GetEntryName(Enum e) {
			uint32_t index = static_cast<uint32_t>(e);
			return Traits::names[index];
		}

		struct Iterator {
			UnderlyingType i = 0;
			const BitsetEnum* bitset;

			bool operator!=(const Iterator& other) const { return i != other.i; }
			void operator++() { ++i; }
			Enum operator*() const { return static_cast<Enum>(i); }
		};

		Iterator begin() const { return { 0, this }; }
		Iterator end()   const { return { Traits::size, this }; }
	};

	template<typename Enum>
	class BitsetFlags {
	public:
		using UnderlyingType = std::underlying_type_t<Enum>;
		using Traits = EnumFlagsTraits<Enum>;
		static constexpr uint32_t bitsPerWord = (CHAR_BIT * sizeof(UnderlyingType));
		static constexpr uint32_t enumBitCount = Traits::size;
		static_assert(std::is_enum_v<Enum>, "BitsetFlags requires an enum type");

		BitsetFlags() = default;
		BitsetFlags(const BitsetFlags&) = default;
		BitsetFlags(BitsetFlags&&) = default;
		BitsetFlags(Enum enumValue) : value(static_cast<UnderlingType>(enumValue)) {}

		BitsetFlags& operator=(const BitsetFlags& other) {
			value = other.value;
			return *this;
		}

		BitsetFlags& operator=(BitsetFlags&& other) {
			value = other.value;
			return *this;
		}

		BitsetFlags& operator=(Enum enumValue) {
			value = static_cast<UnderlyingType>(enumValue);
			return *this;
		}

		void Set() {
			value = ~UnderlyingType(0);
		}

		void Set(uint32_t bitIndex) {
			value |= (1 << bitIndex);
		}

		void Set(Enum e) {
			value |= static_cast<UnderlyingType>(e);
		}

		void Set(uint32_t bitIndex, bool shouldSet) {
			if (shouldSet) {
				value |= (1 << bitIndex);
			}
			else {
				value &= ~(1 << bitIndex);
			}
		}

		void Set(Enum e, bool shouldSet) {
			if (shouldSet) {
				value |= static_cast<UnderlyingType>(e);
			}
			else {
				value &= ~static_cast<UnderlyingType>(e);
			}
		}

		void Unset() {
			value = 0;
		}

		void Unset(uint32_t index) {
			value &= ~(1 << index);
		}

		void Unset(Enum e) {
			value &= ~static_cast<UnderlyingType>(e);
		}

		void Flip() {
			value = ~value;
		}

		void Flip(uint32_t index) {
			value ^= (1 << index);
		}

		void Flip(Enum e) {
			value ^= static_cast<UnderlyingType>(e);
		}

		bool Test(uint32_t index) const {
			return (value >> index) & 1;
		}

		bool Test(Enum e) const {
			return value & static_cast<UnderlyingType>(e);
		}

		static constexpr uint32_t GetBitCount() {
			return Traits::size;
		}

		static constexpr const char* GetFlagNameByIndex(uint32_t index) {
			return Traits::names[index];
		}

		static constexpr const char* GetFlagName(Enum e) {
			uint32_t index = static_cast<uint32_t>(std::log2(static_cast<uint32_t>(e)));
			return Traits::names[index];
		}

		Enum GetValueEnum() const {
			return static_cast<Enum>(value);
		}

		UnderlyingType GetValueUnderlying() const {
			return value;
		}

		bool Any() const {
			return value;
		}

		bool None() const {
			return value == 0;
		}

		BitsetFlags& operator&=(const BitsetFlags& other) const {
			value &= other.value;
			return *this;
		}

		BitsetFlags& operator|=(const BitsetFlags& other) const {
			value |= other.value;
			return *this;
		}

		BitsetFlags& operator^=(const BitsetFlags& other) const {
			value ^= other.value;
			return *this;
		}

		BitsetFlags operator&(const BitsetFlags& other) const {
			return value & other.value;
		}

		BitsetFlags operator|(const BitsetFlags& other) const {
			return value | other.value;
		}

		BitsetFlags operator^(const BitsetFlags& other) const {
			return value ^ other.value;
		}

		BitsetFlags operator~() const {
			return ~value;
		}

		bool operator[](uint32_t index) const {
			return (value >> index) & 1;
		}

		bool operator[](Enum e) const {
			return value & static_cast<UnderlyingType>(e);
		}

		bool operator==(const BitsetFlags& other) const {
			return value == other.value;
		}

		bool operator!=(const BitsetFlags& other) const {
			return value != other.value;
		}

		struct Iterator {
			size_t i = 0;
			const BitsetFlags* bitset;

			bool operator!=(const Iterator& other) const { return i != other.i; }
			void operator++() { ++i; }
			Enum operator*() const { return static_cast<Enum>(i); }
		};

		Iterator begin() const { return { 0, this }; }
		Iterator end()   const { return { EnumTraits<Enum>::size, this }; }

	protected:
		UnderlyingType value = 0;
	};
}

template<uint32_t bitCount, typename WordType = uint32_t>
std::ostream& operator<< (std::ostream& stream, const Grindstone::Containers::Bitset<bitCount, WordType>& x) {
	for (uint32_t i = bitCount; i > 0; --i) {
		stream << (x.Test(i - 1) ? '1' : '0');
	}

	return stream;
}

template<uint32_t bitCount, typename WordType = uint32_t>
std::istream& operator>>(std::istream& stream, Grindstone::Containers::Bitset<bitCount, WordType>& x) {
	std::string str;
	stream >> str;

	if (str.size() > bitCount) {
		stream.setstate(std::ios::failbit);
		return stream;
	}

	x.Unset();

	uint32_t offset = bitCount - static_cast<uint32_t>(str.size());
	for (uint32_t i = 0; i < str.size(); ++i) {
		char c = str[i];
		if (c == '1') {
			x.Set(offset + i);
		}
		else if (c != '0') {
			stream.setstate(std::ios::failbit);
			return stream;
		}
	}

	return stream;
}

template<typename Enum>
std::ostream& operator<< (std::ostream& stream, const Grindstone::Containers::BitsetEnum<Enum>& x) {
	for (uint32_t i = x.GetBitCount(); i > 0; --i) {
		stream << (x.Test(i - 1) ? '1' : '0');
	}

	return stream;
}

template<typename Enum>
std::istream& operator>>(std::istream& stream, Grindstone::Containers::BitsetEnum<Enum>& x) {
	std::string str;
	stream >> str;

	if (str.size() > x.GetBitCount()) {
		stream.setstate(std::ios::failbit);
		return stream;
	}

	x.Unset();

	uint32_t offset = x.GetBitCount() - static_cast<uint32_t>(str.size());
	for (uint32_t i = 0; i < str.size(); ++i) {
		char c = str[i];
		if (c == '1') {
			x.Set(offset + i);
		}
		else if (c != '0') {
			stream.setstate(std::ios::failbit);
			return stream;
		}
	}

	return stream;
}

template<typename Enum>
std::ostream& operator<< (std::ostream& stream, const Grindstone::Containers::BitsetFlags<Enum>& x) {
	for (uint32_t i = x.GetBitCount(); i > 0; --i) {
		stream << (x.Test(i - 1) ? '1' : '0');
	}

	return stream;
}

template<typename Enum>
std::istream& operator>>(std::istream& stream, Grindstone::Containers::BitsetFlags<Enum>& x) {
	std::string str;
	stream >> str;

	if (str.size() > x.GetBitCount()) {
		stream.setstate(std::ios::failbit);
		return stream;
	}

	x.Unset();

	uint32_t offset = x.GetBitCount() - static_cast<uint32_t>(str.size());
	for (uint32_t i = 0; i < str.size(); ++i) {
		char c = str[i];
		if (c == '1') {
			x.Set(offset + i);
		}
		else if (c != '0') {
			stream.setstate(std::ios::failbit);
			return stream;
		}
	}

	return stream;
}
