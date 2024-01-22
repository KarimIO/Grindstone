#pragma once

#include "Assert.hpp"
#include "IntTypes.hpp"

namespace Grindstone {
	class BufferView {
	public:
		BufferView() : bufferPtr(nullptr), size(0) {}
		BufferView(void* bufferPtr, uint64_t size) : bufferPtr(bufferPtr), size(size) {}

		~BufferView() {
			bufferPtr = nullptr;
			size = 0;
		}

		void* Get() {
			return bufferPtr;
		}

		const void* Get() const {
			return bufferPtr;
		}

		template<typename T>
		T* Get() {
			return static_cast<T*>(bufferPtr);
		}

		template<typename T>
		const T* Get() const {
			return static_cast<const T*>(bufferPtr);
		}

		uint64_t GetSize() const {
			return size;
		}

	protected:
		void* bufferPtr = nullptr;
		uint64_t size = 0;
	};

	class Buffer {
	public:
		Buffer() : bufferPtr(nullptr), capacity(0) {}
		Buffer(uint64_t capacity) : capacity(capacity) {
			this->bufferPtr = new Byte[capacity];
		}

		Buffer(void* bufferPtr, uint64_t capacity) : bufferPtr(static_cast<Byte*>(bufferPtr)), capacity(capacity) {}

		// Copy-Constructor
		Buffer(const Buffer& other) {
			capacity = other.capacity;
			bufferPtr = new Byte[capacity];
			memcpy(bufferPtr, other.bufferPtr, capacity);
		}

		// Copy-Constructor
		Buffer(const Buffer&& other) noexcept {
			bufferPtr = other.bufferPtr;
			capacity = other.capacity;
		}

		void ZeroInitialize() {
			if (bufferPtr)
				memset(bufferPtr, 0, capacity);
		}

		virtual BufferView GetBufferView(uint64_t segmentOffset, uint64_t segmentSize) {
			Byte* targetPtr = bufferPtr + segmentOffset;
			if (targetPtr < bufferPtr) {
				GS_ASSERT_LOG("Start of view is before start of buffer.")
				return BufferView();
			}

			if (targetPtr + segmentSize > bufferPtr + capacity) {
				GS_ASSERT_LOG("End of view is after end of buffer.")
				return BufferView();
			}

			return BufferView(targetPtr, segmentSize);
		}

		Byte& operator[](int index) {
			return (static_cast<Byte*>(bufferPtr))[index];
		}

		Byte operator[](int index) const {
			return (static_cast<Byte*>(bufferPtr))[index];
		}

		operator bool() const {
			return bufferPtr != nullptr;
		}

		void Clear() {
			if (bufferPtr != nullptr) {
				delete bufferPtr;
				bufferPtr = nullptr;
			}

			capacity = 0;
		}

		Byte* Get() {
			return bufferPtr;
		}

		const Byte* Get() const {
			return bufferPtr;
		}

		uint64_t GetCapacity() const {
			return capacity;
		}

	protected:
		Byte* bufferPtr = nullptr;
		uint64_t capacity = 0;
	};

	class ResizableBuffer : public Buffer {
	public:
		ResizableBuffer() : Buffer(), currentPtr(nullptr), size(0) {}

		ResizableBuffer(uint64_t capacity) : Buffer() {
			bufferPtr = new Byte[capacity];
			currentPtr = bufferPtr;
			this->capacity = capacity;
		}

		~ResizableBuffer() {
			currentPtr = nullptr;
			size = 0;
		}

		// Copy-Constructor
		ResizableBuffer(const ResizableBuffer& other) {
			capacity = other.capacity;
			size = other.size;

			bufferPtr = new Byte[capacity];
			currentPtr = other.currentPtr;

			memcpy(bufferPtr, other.bufferPtr, size);
		}

		// Copy-Constructor
		ResizableBuffer(const ResizableBuffer&& other) noexcept {
			bufferPtr = other.bufferPtr;
			currentPtr = other.currentPtr;
			capacity = other.capacity;
			size = other.size;
		}

		virtual BufferView GetBufferView(uint64_t segmentOffset, uint64_t segmentSize) override {
			Byte* targetPtr = bufferPtr + segmentOffset;
			if (targetPtr < bufferPtr) {
				GS_ASSERT_LOG("Start of view is before start of buffer.")
				return BufferView();
			}

			if (targetPtr + segmentSize > bufferPtr + size) {
				GS_ASSERT_LOG("End of view is after end of used buffer.")
				return BufferView();
			}

			return BufferView(targetPtr, segmentSize);
		}

		void* AddToBuffer(void* srcPtr, uint64_t srcSize) {
			if (srcPtr == nullptr) {
				GS_ASSERT_LOG("Source memory is nullptr.")
				return nullptr;
			}

			uint64_t spaceLeft = GetSpaceLeft();
			if (srcSize > spaceLeft) {
				GS_ASSERT_LOG("Source memory size is too small to fit.")
				return nullptr;
			}

			memcpy(currentPtr, srcPtr, srcSize);
			Byte* prevPtr = currentPtr;
			currentPtr += srcSize;
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
