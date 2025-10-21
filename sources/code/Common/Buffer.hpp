#pragma once

#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "Assert.hpp"
#include "Containers/Span.hpp"

namespace Grindstone {
	class Buffer {
	public:
		Buffer() : bufferPtr(nullptr), capacity(0) {}
		Buffer(uint64_t newCapacity) : capacity(newCapacity) {
			this->bufferPtr = static_cast<Byte*>(Grindstone::Memory::AllocatorCore::AllocateRaw(capacity, alignof(Buffer), "Buffer"));
		}

		// Copy-Constructor
		Buffer(const Buffer& other) : bufferPtr(nullptr), capacity(other.capacity) {
			bufferPtr = static_cast<Byte*>(Grindstone::Memory::AllocatorCore::AllocateRaw(capacity, alignof(Buffer), "Buffer"));
			memcpy(bufferPtr, other.bufferPtr, capacity);
		}

		// Move-Constructor
		Buffer(Buffer&& other) noexcept : capacity(other.capacity), bufferPtr(other.bufferPtr) {
			other.bufferPtr = nullptr;
			other.capacity = 0;
		}

		~Buffer() {
			if (bufferPtr != nullptr) {
				Grindstone::Memory::AllocatorCore::Free(bufferPtr);
				bufferPtr = nullptr;
			}
			capacity = 0;
		}

		void ZeroInitialize() {
			if (bufferPtr) {
				memset(bufferPtr, 0, capacity);
			}
		}

		[[nodiscard]] virtual Grindstone::Containers::BufferSpan GetSpan(uint64_t segmentOffset, uint64_t segmentSize) {
			Byte* targetPtr = bufferPtr + segmentOffset;
			if (targetPtr < bufferPtr) {
				GS_ASSERT_ENGINE("Start of span is before start of buffer.");
				return {};
			}

			if (targetPtr + segmentSize > bufferPtr + capacity) {
				GS_ASSERT_ENGINE("End of span is after end of buffer.");
				return {};
			}

			return { targetPtr, segmentSize };
		}

		[[nodiscard]] virtual Grindstone::Containers::BufferSpan GetSpan() {
			return { bufferPtr, capacity };
		}

		template<typename T>
		[[nodiscard]] Grindstone::Containers::Span<T> GetSpan(uint64_t offset, uint64_t count) {
			Byte* bytePtr = bufferPtr + offset;
			T* targetPtr = reinterpret_cast<T*>(bytePtr);
			if (bytePtr < bufferPtr) {
				GS_ASSERT_ENGINE("Start of span is before start of buffer.");
				return {};
			}

			if (reinterpret_cast<Byte*>(bytePtr + (count * sizeof(T))) > bufferPtr + capacity) {
				GS_ASSERT_ENGINE("End of span is after end of buffer.");
				return {};
			}

			return { targetPtr, count };
		}

		template<typename T>
		[[nodiscard]] T* Get(uint64_t offset) {
			Byte* bytePtr = bufferPtr + offset;
			T* targetPtr = reinterpret_cast<T*>(bytePtr);
			if (bytePtr < bufferPtr) {
				GS_ASSERT_ENGINE("Start of span is before start of buffer.");
				return nullptr;
			}

			if (reinterpret_cast<Byte*>(bytePtr + sizeof(T)) > bufferPtr + capacity) {
				GS_ASSERT_ENGINE("End of span is after end of buffer.");
				return nullptr;
			}

			return targetPtr;
		}

		template<typename T>
		[[nodiscard]] const T* Get(uint64_t offset) const {
			Byte* bytePtr = bufferPtr + offset;
			T* targetPtr = reinterpret_cast<T*>(bytePtr);
			if (bytePtr < bufferPtr) {
				GS_ASSERT_ENGINE("Start of span is before start of buffer.");
				return nullptr;
			}

			if (reinterpret_cast<Byte*>(bytePtr + sizeof(T)) > bufferPtr + capacity) {
				GS_ASSERT_ENGINE("End of span is after end of buffer.");
				return nullptr;
			}

			return targetPtr;
		}

		[[nodiscard]] Byte* Get(uint64_t offset) {
			Byte* bytePtr = bufferPtr + offset;
			if (bytePtr < bufferPtr) {
				GS_ASSERT_ENGINE("Offset is before start of buffer.");
				return nullptr;
			}

			if (reinterpret_cast<Byte*>(bytePtr) >= bufferPtr + capacity) {
				GS_ASSERT_ENGINE("Offset is at or after end of buffer.");
				return nullptr;
			}

			return bytePtr;
		}

		[[nodiscard]] Byte* Get(uint64_t offset) const {
			Byte* bytePtr = bufferPtr + offset;
			if (bytePtr < bufferPtr) {
				GS_ASSERT_ENGINE("Offset is before start of buffer.");
				return nullptr;
			}

			if (reinterpret_cast<Byte*>(bytePtr) >= bufferPtr + capacity) {
				GS_ASSERT_ENGINE("Offset is at or after end of buffer.");
				return nullptr;
			}

			return bytePtr;
		}

		Buffer& operator=(const Buffer& other) {
			if(this == &other) {
				return *this;
			}

			capacity = other.capacity;
			bufferPtr = static_cast<Byte*>(Grindstone::Memory::AllocatorCore::AllocateRaw(capacity, alignof(Buffer), "Buffer"));
			memcpy(bufferPtr, other.bufferPtr, capacity);
			return *this;
		}

		Buffer& operator=(Buffer&& other) noexcept {
			if (this == &other) {
				return *this;
			}

			if (bufferPtr) {
				Grindstone::Memory::AllocatorCore::Free(bufferPtr);
			}

			bufferPtr = other.bufferPtr;
			capacity = other.capacity;

			other.bufferPtr = nullptr;
			other.capacity = 0;

			return *this;
		}

		[[nodiscard]] Byte& operator[](int index) {
			return bufferPtr[index];
		}

		[[nodiscard]] Byte operator[](int index) const {
			return bufferPtr[index];
		}

		explicit operator bool() const {
			return bufferPtr != nullptr;
		}

		void Clear() {
			if (bufferPtr != nullptr) {
				Grindstone::Memory::AllocatorCore::Free(bufferPtr);
				bufferPtr = nullptr;
			}

			capacity = 0;
		}

		[[nodiscard]] Byte* Get() {
			return bufferPtr;
		}

		[[nodiscard]] const Byte* Get() const {
			return bufferPtr;
		}

		[[nodiscard]] uint64_t GetCapacity() const {
			return capacity;
		}

		[[nodiscard]] static Buffer MakeCopiedBuffer(void* srcBufferPtr, const uint64_t capacity) {
			Byte* bufferPtr = static_cast<Byte*>(Grindstone::Memory::AllocatorCore::AllocateRaw(capacity, alignof(Buffer), "Buffer"));
			memcpy(bufferPtr, srcBufferPtr, capacity);
			return Buffer( bufferPtr, capacity );
		}

		[[nodiscard]] static Buffer MakeMovedBuffer(void* srcBufferPtr, const uint64_t capacity) {
			Byte* bufferPtr = static_cast<Byte*>(srcBufferPtr);
			return Buffer( bufferPtr, capacity );
		}

	protected:
		Buffer(void* bufferPtr, const uint64_t capacity) : bufferPtr(static_cast<Byte*>(bufferPtr)), capacity(capacity) {}

		Byte* bufferPtr = nullptr;
		uint64_t capacity = 0;
	};

	class ResizableBuffer : public Buffer {
	public:
		ResizableBuffer() : Buffer(), currentPtr(nullptr), size(0) {}

		ResizableBuffer(uint64_t capacity) : Buffer() {
			bufferPtr = static_cast<Byte*>(Grindstone::Memory::AllocatorCore::AllocateRaw(capacity, alignof(ResizableBuffer), "ResizableBuffer"));
			currentPtr = bufferPtr;
			this->capacity = capacity;
		}

		virtual ~ResizableBuffer() {
			currentPtr = nullptr;
			size = 0;
		}

		// Copy-Constructor
		ResizableBuffer(const ResizableBuffer& other) {
			capacity = other.capacity;
			size = other.size;

			bufferPtr = static_cast<Byte*>(Grindstone::Memory::AllocatorCore::AllocateRaw(capacity, alignof(ResizableBuffer), "ResizableBuffer"));
			currentPtr = other.currentPtr;

			memcpy(bufferPtr, other.bufferPtr, size);
		}

		// Copy-Constructor
		ResizableBuffer(ResizableBuffer&& other) noexcept {
			bufferPtr = other.bufferPtr;
			currentPtr = other.currentPtr;
			capacity = other.capacity;
			size = other.size;

			other.bufferPtr = nullptr;
			other.currentPtr = nullptr;
			other.capacity = 0;
			other.size = 0;
		}

		virtual Grindstone::Containers::BufferSpan GetSpan(uint64_t segmentOffset, uint64_t segmentSize) override {
			Byte* targetPtr = bufferPtr + segmentOffset;
			if (targetPtr < bufferPtr) {
				GS_ASSERT_ENGINE("Start of span is before start of buffer.")
				return Grindstone::Containers::BufferSpan();
			}

			if (targetPtr + segmentSize > bufferPtr + size) {
				GS_ASSERT_ENGINE("End of span is after end of used buffer.")
				return Grindstone::Containers::BufferSpan();
			}

			return Grindstone::Containers::BufferSpan(targetPtr, segmentSize);
		}

		template<typename T>
		Grindstone::Containers::Span<T> GetSpan(uint64_t offset, uint64_t count) {
			T* targetPtr = reinterpret_cast<T*>(bufferPtr + offset);
			if (targetPtr < bufferPtr) {
				GS_ASSERT_ENGINE("Start of span is before start of buffer.");
				return {};
			}

			if (targetPtr + count > bufferPtr + capacity) {
				GS_ASSERT_ENGINE("End of span is after end of buffer.");
				return {};
			}

			return { targetPtr, count };
		}

		void* AddToBuffer(const void* srcPtr, uint64_t srcSize) {
			if (srcPtr == nullptr) {
				GS_ASSERT_ENGINE("Source memory is nullptr.")
				return nullptr;
			}

			uint64_t spaceLeft = GetSpaceLeft();
			if (srcSize > spaceLeft) {
				GS_ASSERT_ENGINE("Source memory size is too small to fit.")
				return nullptr;
			}

			memcpy(currentPtr, srcPtr, srcSize);
			Byte* prevPtr = currentPtr;
			currentPtr += srcSize;
			size += srcSize;
			return prevPtr;
		}

		uint64_t GetSpaceLeft() const {
			return capacity - size;
		}

		uint64_t GetUsedSize() const {
			return size;
		}

	protected:
		Byte* currentPtr = nullptr;
		uint64_t size = 0;
	};
}
