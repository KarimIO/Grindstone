#pragma once

#include <type_traits>
#include "../Assert.hpp"

namespace Grindstone::Containers {
	template<typename T>
	struct ConstArrayIterator {
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;

		ConstArrayIterator(pointer ptr) : ptr(ptr) {}

		const reference operator*() const { return *ptr; }
		const pointer operator->() const { return ptr; }
		ConstArrayIterator& operator++() { ptr++; return *this; }
		ConstArrayIterator operator++(int) { ConstArrayIterator tmp = *this; ++(*this); return tmp; }
		friend bool operator== (const ConstArrayIterator& a, const ConstArrayIterator& b) { return a.ptr == b.ptr; };
		friend bool operator!= (const ConstArrayIterator& a, const ConstArrayIterator& b) { return a.ptr != b.ptr; };

	private:
		pointer ptr;
	};

	template<typename T>
	struct ArrayIterator {
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;

		ArrayIterator(pointer ptr) : ptr(ptr) {}

		reference operator*() const { return *ptr; }
		pointer operator->() { return ptr; }
		ArrayIterator& operator++() { ptr++; return *this; }
		ArrayIterator operator++(int) { ArrayIterator tmp = *this; ++(*this); return tmp; }
		ArrayIterator& operator--() { ptr--; return *this; }
		ArrayIterator operator--(int) { ArrayIterator tmp = *this; --(*this); return tmp; }
		ArrayIterator operator+(int offset) { ptr += offset;  return *this; }
		ArrayIterator operator-(int offset) { ptr -= offset;  return *this; }
		friend bool operator== (const ArrayIterator& a, const ArrayIterator& b) { return a.ptr == b.ptr; };
		friend bool operator!= (const ArrayIterator& a, const ArrayIterator& b) { return a.ptr != b.ptr; };
		friend bool operator<(const ArrayIterator& l, const ArrayIterator& r) { return &(*l) < &(*r); }
		friend bool operator>(const ArrayIterator& l, const ArrayIterator& r) { return r < l; }
		friend bool operator<=(const ArrayIterator& l, const ArrayIterator& r) { return !(r < l); }
		friend bool operator>=(const ArrayIterator& l, const ArrayIterator& r) { return !(l < r); }

	protected:
		pointer ptr;
	};

	template<typename T>
	struct ConstReverseArrayIterator {
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;

		ConstReverseArrayIterator(pointer ptr) : ptr(ptr) {}

		const reference operator*() const { return *ptr; }
		const pointer operator->() const { return ptr; }
		ConstReverseArrayIterator& operator++() { ptr--; return *this; }
		ConstReverseArrayIterator operator++(int) { ConstReverseArrayIterator tmp = *this; --(*this); return tmp; }
		friend bool operator== (const ConstReverseArrayIterator& a, const ConstReverseArrayIterator& b) { return a.ptr == b.ptr; };
		friend bool operator!= (const ConstReverseArrayIterator& a, const ConstReverseArrayIterator& b) { return a.ptr != b.ptr; };

	private:
		pointer ptr;
	};

	template<typename T>
	struct ReverseArrayIterator {
		using iterator_category = std::forward_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using reference = T&;

		ReverseArrayIterator(pointer ptr) : ptr(ptr) {}

		reference operator*() const { return *ptr; }
		pointer operator->() { return ptr; }
		ReverseArrayIterator& operator++() { ptr--; return *this; }
		ReverseArrayIterator operator++(int) { ReverseArrayIterator tmp = *this; --(*this); return tmp; }
		friend bool operator== (const ReverseArrayIterator& a, const ReverseArrayIterator& b) { return a.ptr == b.ptr; };
		friend bool operator!= (const ReverseArrayIterator& a, const ReverseArrayIterator& b) { return a.ptr != b.ptr; };

	private:
		pointer ptr;
	};
}
