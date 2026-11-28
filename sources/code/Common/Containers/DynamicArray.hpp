#pragma once

#include <Common/Assert.hpp>
#include <Common/Memory/Allocators/AllocatorConcept.hpp>
#include <Common/Memory/Allocators/DynamicAllocator.hpp>
#include "Iterators.hpp"
#include "Span.hpp"

namespace Grindstone::Containers {
	template<typename T, Grindstone::Memory::Allocators::Allocator AllocatorType = Grindstone::Memory::Allocators::DynamicAllocator>
	class DynamicArray {
	public:
		using Iterator = ArrayIterator<T>;
		using ConstIterator = ConstArrayIterator<T>;
		using ReverseIterator = ReverseArrayIterator<T>;
		using ConstReverseIterator = ConstArrayIterator<T>;

		DynamicArray() = default;

		DynamicArray(AllocatorType* allocator, std::initializer_list<T> list) :
			allocator(allocator),
			capacity(GetStaticRequestCapacity(list.size())),
			size(list.size()),
			contents(Allocate(capacity)) {
			size_t i = 0;
			for (auto val : list) {
				new (&contents[i++]) T(val);
			}
		}

		DynamicArray(AllocatorType* allocator, T* data, size_t size) :
			allocator(allocator),
			capacity(GetStaticRequestCapacity(size)),
			size(size),
			contents(Allocate(size)) {
			size_t i = 0;
			for (size_t i = 0; i < size; ++i) {
				new (&contents[i]) T(data[i]);
			}
		}

		DynamicArray(const DynamicArray& other) :
			allocator(other.allocator), capacity(0), size(0), contents(nullptr) {
			if (other.size > 0) {
				capacity = other.size;
				contents = Allocate(capacity);
				GS_ASSERT_ENGINE_WITH_MESSAGE(contents != nullptr, "Unable to allocate memory for DynamicArray.");
				for (size_t i = 0; i < other.size; ++i) {
					new (&contents[i]) T(other.contents[i]);
					++size; // keeps the destructor correct if a copy throws
				}
			}
		}


		DynamicArray(DynamicArray&& other) noexcept : allocator(other.allocator),
			capacity(other.capacity),
			size(other.size),
			contents(other.contents) {

			other.size = 0;
			other.capacity = 0;
			other.contents = nullptr;
		}

		DynamicArray& operator=(const DynamicArray& other) {
			if (this != &other) {
				DynamicArray copy(other);
				Swap(copy);	 // Copy's destructor cleans up our old contents
			}
			return *this;
		}

		DynamicArray& operator=(DynamicArray&& other) noexcept {
			if (this != &other) {
				Clear();
				allocator = other.allocator;
				size = other.size;
				capacity = other.capacity;
				contents = other.contents;
				other.size = 0;
				other.capacity = 0;
				other.contents = nullptr;
			}
			return *this;
		}

		void Swap(DynamicArray& other) noexcept {
			std::swap(allocator, other.allocator);
			std::swap(capacity, other.capacity);
			std::swap(size, other.size);
			std::swap(contents, other.contents);
		}

		void Clear() {
			if (contents != nullptr) {
				allocator->FreeArray(contents, size);
				contents = nullptr;
			}
			capacity = 0;
			size = 0;
		}

		operator Grindstone::Containers::Span<T>() {
			return Grindstone::Containers::Span<T>{ contents, size };
		}

		Grindstone::Containers::Span<T> GetSpan(size_t index, size_t size) {
			return Grindstone::Containers::Span<T>{ &contents[index], size };
		}

		~DynamicArray() {
			if (contents != nullptr) {
				for (size_t i = 0; i < size; ++i) {
					contents[i].~T();
				}

				allocator->Free(contents);
				contents = nullptr;
			}

			capacity = 0;
			size = 0;
		}

		static DynamicArray CreateExactReserved(size_t initialCapacity) {
			return std::move(DynamicArray(initialCapacity, 0));
		}

		static DynamicArray CreateExactInitialized(size_t initialSize) {
			return std::move(DynamicArray(initialSize, initialSize));
		}

		static DynamicArray CreateReservedWithAtLeast(size_t minimumInitialCapacity) {
			return std::move(DynamicArray(GetStaticRequestCapacity(minimumInitialCapacity), 0));
		}

		static DynamicArray CreateInitializedWithAtLeast(size_t initialSize) {
			return std::move(DynamicArray(GetStaticRequestCapacity(initialSize), initialSize));
		}

		void Resize(size_t newSize) {
			if (newSize < size) {
				for (size_t i = newSize; i < size; ++i) {
					contents[i].~T();
				}
				size = newSize;
				return;
			}

			if (newSize > capacity) {
				size_t newCapacity = GetRequestCapacity(newSize);
				ResizeBuffer(newCapacity);
			}

			for (size_t i = size; i < newSize; ++i) {
				new (&contents[i]) T();
			}
			size = newSize;
		}

		void ReserveToExact(size_t newCapacity) {
			if (newCapacity > capacity) {
				ResizeBuffer(newCapacity);
			}
		}

		void ReserveToAtLeast(size_t newCapacity) {
			if (newCapacity > capacity) {
				ResizeBuffer(GetRequestCapacity(newCapacity));
			}
		}

		void ReserveMoreExact(size_t addedCapacity) {
			ResizeBuffer(capacity + addedCapacity);
		}

		void ReserveMoreAtLeast(size_t addedCapacity) {
			ResizeBuffer(GetRequestCapacity(capacity + addedCapacity));
		}

		T& PushBack(T&& val) {
			IncrementAndResize();
			T& newValue = *new (&contents[size]) T(std::move(val));
			++size;
			return newValue;
		}

		T& PushBack(const T& val) {
			IncrementAndResize();
			T& newValue = contents[size];
			new(&newValue) T(val);
			++size;
			return newValue;
		}

		template<typename... Args>
		T& EmplaceBack(Args&&... args) {
			IncrementAndResize();
			T& newValue = contents[size];
			++size;
			new (&newValue) T(std::forward<Args>(args)...);
			return newValue;
		}

		void AppendList(std::initializer_list<T> list, size_t offset) {
			GS_ASSERT_ENGINE_WITH_MESSAGE(offset <= size, "Insert offset out of range.");
			const size_t count = list.size(), oldSize = size;
			if (count == 0) {
				return;
			}

			if (oldSize + count > capacity) {
				ResizeBuffer(GetRequestCapacity(oldSize + count));
			}

			// Shift the tail up. Slots at or beyond oldSize are raw memory, the rest are live objects.
			for (size_t i = oldSize; i-- > offset; ) {
				T* dst = &contents[i + count];
				if (i + count >= oldSize) new (dst) T(std::move(contents[i]));
				else *dst = std::move(contents[i]);
			}

			size_t i = offset;
			for (const T& v : list) {
				T* dst = &contents[i];
				if (i < oldSize) *dst = v; else new (dst) T(v);
				++i;
			}

			size = oldSize + count;
		}

		void AppendListBack(std::initializer_list<T> list) {
			AppendList(list, size);
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

		void Remove(size_t first, size_t last) {
			GS_ASSERT_ENGINE_WITH_MESSAGE(first <= last && last < size, "Remove range invalid.");

			const size_t count = last - first + 1;
			for (size_t i = last + 1; i < size; ++i) {
				contents[i - count] = std::move(contents[i]);
			}

			for (size_t i = size - count; i < size; ++i) {
				contents[i].~T();
			}

			size -= count;
		}

		void Remove(T& first, T& last) { Remove(size_t(&first - contents), size_t(&last - contents)); }
		void Remove(T& first)          { Remove(first, first); }
		void Remove(size_t index)      { Remove(index, index); }

		bool TryGet(T& outValue, size_t index) {
			if (index < size) {
				outValue = contents[index];
				return true;
			}

			return false;
		}

		bool IsEmpty() const {
			return size == 0;
		}

		size_t GetSize() const {
			return size;
		}

		size_t GetCapacity() const {
			return capacity;
		}

		bool operator==(const Grindstone::Containers::DynamicArray<T, AllocatorType>& other) const {
			if (size != other.GetSize()) {
				return false;
			}

			for (uint32_t i = 0; i < size; ++i) {
				const auto& a = (*this)[i];
				const auto& b = other.Get(i);
				if (a != b) {
					return false;
				}
			}

			return true;
		}

		T& operator[](size_t index) {
			GS_ASSERT_ENGINE_WITH_MESSAGE(index < size, "Array index is invalid.");
			return contents[index];
		}

		const T& operator[](size_t index) const {
			GS_ASSERT_ENGINE_WITH_MESSAGE(index < size, "Array index is invalid.");
			return contents[index];
		}

		T& Get(size_t index) {
			GS_ASSERT_ENGINE_WITH_MESSAGE(index < size, "Array index is invalid.");
			return contents[index];
		}

		const T& Get(size_t index) const {
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
			return ConstIterator(&contents[size]);
		}

		[[nodiscard]] constexpr ReverseIterator rbegin() noexcept {
			return ReverseIterator(&contents[size - 1]);
		}

		[[nodiscard]] constexpr ConstReverseIterator rbegin() const noexcept {
			return ConstReverseIterator(&contents[size - 1]);
		}

		[[nodiscard]] constexpr ReverseIterator rend() noexcept {
			return ReverseIterator(&contents[-1]);
		}

		[[nodiscard]] constexpr ConstReverseIterator rend() const noexcept {
			return ConstReverseIterator(&contents[-1]);
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
			return ConstReverseIterator(&contents[-1]);
		}
	protected:
		DynamicArray(size_t initialCapacity, size_t initialSize) :
			capacity(initialCapacity),
			size(initialSize),
			contents(Allocate(initialCapacity)) {
			for (size_t i = 0; i < initialSize; ++i) {
				new (&contents[i]) T();
			}
		}

		void IncrementAndResize() {
			if (size + 1 > capacity) {
				ResizeBuffer(GetPushOneCapacity());
			}
		}

		void ResizeBuffer(size_t newCapacity) {
			if (capacity == 0) {
				contents = Allocate(newCapacity);
				GS_ASSERT_ENGINE_WITH_MESSAGE(contents != nullptr, "Unable to allocate memory for DynamicArray.");
				capacity = newCapacity;
			}
			else {
				void* newContents = allocator->ReallocRaw(contents, sizeof(T) * newCapacity, alignof(T), "DynamicArray<>");
				GS_ASSERT_ENGINE_WITH_MESSAGE(newContents != nullptr, "Unable to allocate memory for DynamicArray.");
				contents = reinterpret_cast<T*>(newContents);
				capacity = newCapacity;
			}
		}

		size_t GetPushOneCapacity() const {
			if (capacity == 0) {
				return (64 / sizeof(T) > 1) ? (64 / sizeof(T)) : 1;
			}

			if (capacity > 4096 * 32 / sizeof(T)) {
				return capacity * 2;
			}

			return (capacity * 3 + 1) / 2;
		}

		size_t GetRequestCapacity(size_t requestedCapacity) const {
			size_t testCapacity = capacity;

			if (testCapacity == 0) {
				testCapacity = (64 / sizeof(T) > 1) ? (64 / sizeof(T)) : 1;
			}

			while (testCapacity < requestedCapacity) {
				if (testCapacity > 4096 * 32 / sizeof(T)) {
					testCapacity = testCapacity * 2;
				}
				else {
					testCapacity = (testCapacity * 3 + 1) / 2;
				}
			}

			return testCapacity;
		}

		static size_t GetStaticRequestCapacity(size_t requestedCapacity) {
			size_t testCapacity = (64 / sizeof(T) > 1) ? (64 / sizeof(T)) : 1;

			while (testCapacity < requestedCapacity) {
				if (testCapacity > 4096 * 32 / sizeof(T)) {
					testCapacity = testCapacity * 2;
				}
				else {
					testCapacity = (testCapacity * 3 + 1) / 2;
				}
			}

			return testCapacity;
		}

		static void Construct(T* ptr, size_t count) {
			std::uninitialized_value_construct_n(ptr, count);
		}

		inline T* Allocate(size_t cap) {
			return reinterpret_cast<T*>(allocator->AllocateRaw(sizeof(T) * cap, alignof(T), "DynamicArray<>"));
		}

		AllocatorType* allocator = nullptr;
		size_t capacity = 0;
		size_t size = 0;
		T* contents = nullptr;
	};
}
