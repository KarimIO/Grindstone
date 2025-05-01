#pragma once

#include <type_traits>
#include "../Assert.hpp"

#include "Iterators.hpp"

namespace Grindstone::Containers {
	template<typename T>
	class Span {
	public:
		using Iterator = ArrayIterator<T>;
		using ConstIterator = ConstArrayIterator<T>;
		using ReverseIterator = ReverseArrayIterator<T>;
		using ConstReverseIterator = ConstArrayIterator<T>;

		Span() = default;

		Span(T* ptr, size_t size) :
			contents(ptr),
			size(size) {}

		Span(const Span& other) : size(other.size), contents(other.contents) {}

		Span(Span&& other) noexcept : size(other.size), contents(other.contents) {
			other.size = 0;
			other.capacity = 0;
			other.contents = nullptr;
		}

		Span& operator=(const Span& other) {
			size = other.size;
			contents = other.contents;
		}

		Span& operator=(Span&& other) {
			size = other.size;
			contents = other.contents;

			other.size = 0;
			other.contents = nullptr;
		}

		~Span() {
			contents = nullptr;
			size = 0;
		}

		[[nodiscard]] const T& GetBegin() const {
			return contents[0];
		}

		[[nodiscard]] T& GetBegin() {
			return contents[0];
		}

		[[nodiscard]] const T& GetEnd() const {
			return contents[size];
		}

		[[nodiscard]] T& GetEnd() {
			return contents[size];
		}

		bool TryGet(T& outValue, size_t index) {
			if (index < size) {
				contents[index];
				return true;
			}

			return false;
		}

		size_t GetSize() const {
			return size;
		}

		T& operator[](size_t index) {
			GS_ASSERT_ENGINE_WITH_MESSAGE(index < size, "Array index is invalid.");
			return contents[index];
		}

		const T& operator[](size_t index) const {
			GS_ASSERT_ENGINE_WITH_MESSAGE(index < size, "Array index is invalid.");
			return contents[index];
		}

		[[nodiscard]] constexpr Iterator begin() noexcept {
			return Iterator(contents);
		}

		[[nodiscard]] constexpr ConstIterator begin() const noexcept {
			return ConstIterator(contents);
		}

		[[nodiscard]] constexpr Iterator end() noexcept {
			return Iterator(&contents[size]);
		}

		[[nodiscard]] constexpr ConstIterator end() const noexcept {
			return ConstIterator(&contents[size - 1]);
		}

		[[nodiscard]] constexpr ReverseIterator rbegin() noexcept {
			return ReverseIterator(&contents[size - 1]);
		}

		[[nodiscard]] constexpr ConstReverseIterator rbegin() const noexcept {
			return ConstReverseIterator(&contents[size - 1]);
		}

		[[nodiscard]] constexpr ReverseIterator rend() noexcept {
			return ReverseIterator(contents - 1);
		}

		[[nodiscard]] constexpr ConstReverseIterator rend() const noexcept {
			return ConstReverseIterator(contents - 1);
		}

		[[nodiscard]] constexpr ConstIterator cbegin() const noexcept {
			return ConstIterator(contents);
		}

		[[nodiscard]] constexpr ConstIterator cend() const noexcept {
			return ConstIterator(&contents[size]);
		}

		[[nodiscard]] constexpr ConstReverseIterator crbegin() const noexcept {
			return ConstReverseIterator(&contents[size - 1]);
		}

		[[nodiscard]] constexpr ConstReverseIterator crend() const noexcept {
			return ConstReverseIterator(contents - 1);
		}
	protected:
		size_t capacity = 0;
		size_t size = 0;
		T* contents = nullptr;
	};

	template<typename T>
	class ReverseRange {
	public:
		explicit ReverseRange(T& iterable) : iterable{ iterable } {}
		auto begin() const { return std::rbegin(iterable); }
		auto end() const { return std::rend(iterable); }
	private:
		T& iterable;
	};

	template<typename T>
	class ReverseRangeTemp {
	public:
		explicit ReverseRangeTemp(T&& iterable) : iterable{ std::move(iterable) } {}
		auto begin() const { return std::rbegin(iterable); }
		auto end() const { return std::rend(iterable); }
	private:
		T iterable;
	};
}
