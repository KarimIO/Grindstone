#pragma once

#include <EngineCore/Utils/MemoryAllocator.hpp>

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
			this->bufferPtr = static_cast<Byte*>(Grindstone::Memory::AllocatorCore::AllocateRaw(capacity, alignof(Buffer), "Buffer"));
		}

		Buffer(void* bufferPtr, const uint64_t capacity) : bufferPtr(static_cast<Byte*>(bufferPtr)), capacity(capacity) {}

		// Copy-Constructor
		Buffer(const Buffer& other) : capacity(other.capacity) {
			bufferPtr = static_cast<Byte*>(Grindstone::Memory::AllocatorCore::AllocateRaw(capacity, alignof(Buffer), "Buffer"));
			memcpy(bufferPtr, other.bufferPtr, capacity);
		}

		// Move-Constructor
		Buffer(Buffer&& other) noexcept : capacity(other.capacity), bufferPtr(bufferPtr) {
			bufferPtr = other.bufferPtr;
			capacity = other.capacity;

			other.bufferPtr = nullptr;
			other.capacity = 0;
		}

		~Buffer() {
			Grindstone::Memory::AllocatorCore::Free(bufferPtr);
			bufferPtr = nullptr;
			capacity = 0;
		}

		void ZeroInitialize() {
			if (bufferPtr) {
				memset(bufferPtr, 0, capacity);
			}
		}

		virtual BufferView GetBufferView(uint64_t segmentOffset, uint64_t segmentSize) {
			Byte* targetPtr = bufferPtr + segmentOffset;
			if (targetPtr < bufferPtr) {
				GS_ASSERT_ENGINE("Start of view is before start of buffer.")
				return {};
			}

			if (targetPtr + segmentSize > bufferPtr + capacity) {
				GS_ASSERT_ENGINE("End of view is after end of buffer.")
				return {};
			}

			return {targetPtr, segmentSize};
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

		Byte& operator[](int index) {
			return bufferPtr[index];
		}

		Byte operator[](int index) const {
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

		virtual BufferView GetBufferView(uint64_t segmentOffset, uint64_t segmentSize) override {
			Byte* targetPtr = bufferPtr + segmentOffset;
			if (targetPtr < bufferPtr) {
				GS_ASSERT_ENGINE("Start of view is before start of buffer.")
				return BufferView();
			}

			if (targetPtr + segmentSize > bufferPtr + size) {
				GS_ASSERT_ENGINE("End of view is after end of used buffer.")
				return BufferView();
			}

			return BufferView(targetPtr, segmentSize);
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
