#pragma once

#include <type_traits>

#include "../Memory/Allocators/AllocatorConcept.hpp"
#include "../Assert.hpp"
#include "Iterators.hpp"
#include "Span.hpp"

namespace Grindstone::Containers {
	template<typename T, Allocator AllocatorType>
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
			contents(Allocate(sizeof(T) * capacity)),
			size(list.size()) {
			size_t i = 0;
			for (auto val : list) {
				new (&contents[i++]) T(val);
			}
		}

		DynamicArray(const DynamicArray& other) :
			allocator(other.allocator),
			size(other.size),
			capacity(other.capacity),
			contents(nullptr) {
			if (other.contents != nullptr) {
				contents = Allocate(sizeof(T) * capacity);
				for (size_t i = 0; i < size; ++i) {
					new (&contents[i]) T(other.contents[i]);
				}
				GS_ASSERT_ENGINE_WITH_MESSAGE(contents != nullptr, "Unable to allocate memory for DynamicArray.");
			}
		}

		DynamicArray(DynamicArray&& other) noexcept : allocator(other.allocator), size(other.size), capacity(other.capacity), contents(other.contents) {
			size = other.size;
			capacity = other.capacity;
			contents = other.contents;

			other.size = 0;
			other.capacity = 0;
			other.contents = nullptr;
		}

		DynamicArray& operator=(const DynamicArray& other) {
			if (contents != nullptr) {
				for (size_t i = 0; i < size; ++i) {
					contents[i].~T();
				}

				allocator->Free(contents);
				contents = nullptr;
			}

			allocator = other.allocator;
			size = other.size;
			capacity = other.capacity;
			if (other.contents == nullptr) {
				contents = nullptr;
			}
			else {
				contents = Allocate(sizeof(T) * capacity);
				for (size_t i = 0; i < size; ++i) {
					new (&contents[i]) T(std::move(other.contents[i]));
				}
				GS_ASSERT_ENGINE_WITH_MESSAGE(contents != nullptr, "Unable to allocate memory for DynamicArray.");
			}
		}

		DynamicArray& operator=(DynamicArray&& other) {
			if (contents != nullptr) {
				for (size_t i = 0; i < size; ++i) {
					contents[i].~T();
				}

				allocator->Free(contents);
				contents = nullptr;
			}

			allocator = other.allocator;
			size = other.size;
			capacity = other.capacity;
			contents = other.contents;

			other.size = 0;
			other.capacity = 0;
			other.contents = nullptr;
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
			T& newValue = contents[size];
			newValue = std::move(val);
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
			size_t currentSize = size;
			size_t countToAdd = list.size();
			size += countToAdd;
			if (size >= capacity) {
				size_t newCapacity = GetRequestCapacity(size);
				ResizeBuffer(newCapacity);
			}

			for (size_t index = currentSize - 1; index >= offset; --index) {
				new (&contents[index + countToAdd]) T(std::move(contents[index]));
			}

			size_t i = offset;
			for (auto& val : list) {
				new (&contents[i++]) T(std::move(val));
			}
		}

		void AppendListBack(std::initializer_list<T> list) {
			size_t currentSize = size;
			size += list.size();
			size_t newCapacity = GetRequestCapacity(size);
			ResizeBuffer(newCapacity);

			size_t i = currentSize;
			for (auto val : list) {
				new (&contents[i++]) T(val);
			}
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

		void Remove(T& first, T& last) {
			size_t firstIndex = (&first - contents);
			size_t lastIndex = (&last - contents);
			for (size_t index = firstIndex; index <= lastIndex; ++index) {
				contents[index].~T();
			}

			for (size_t index = 0; index < size - lastIndex - 1; ++index) {
				contents[firstIndex + index] = std::move(contents[lastIndex + index + 1]);
			}
			
			size -= 1 + lastIndex - firstIndex;
		}

		void Remove(T& first) {
			Remove(first, first);
		}

		void Remove(size_t index) {
			Remove(contents[index], contents[index]);
		}

		bool TryGet(T& outValue, size_t index) {
			if (index < size) {
				contents[index];
				return true;
			}

			return false;
		}

		size_t GetSize() {
			return size;
		}

		size_t GetCapacity() {
			return capacity;
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
		DynamicArray(size_t initialCapacity, size_t initialSize) :
			capacity(initialCapacity),
			size(initialSize),
			contents(Allocate(sizeof(T) * initialCapacity)) {
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
				contents = Allocate(sizeof(T) * newCapacity);
				GS_ASSERT_ENGINE_WITH_MESSAGE(contents != nullptr, "Unable to allocate memory for DynamicArray.");
				capacity = newCapacity;
			}
			else {
				// TODO: Switch to realloc
				void* newContents = allocator->Reallocate(contents, sizeof(T) * newCapacity, alignof(T), "DynamicArray<>");
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

		inline T* Allocate(size_t cap) {
			return reinterpret_cast<T*>(allocator->AllocateRaw(sizeof(T) * capacity, alignof(T), "DynamicArray<>"));
		}

		AllocatorType* allocator = nullptr;
		size_t capacity = 0;
		size_t size = 0;
		T* contents = nullptr;
	};
}
