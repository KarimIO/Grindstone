#pragma once

#include <stdint.h>

#include <Common/Containers/Bitset.hpp>

#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	enum class BufferUsage : uint8_t {
		Vertex = 1,
		Index = 1 << 1,
		Uniform = 1 << 2,
		Storage = 1 << 3,
		Indirect = 1 << 4,
		TransferSrc = 1 << 5,
		TransferDst = 1 << 6
	};
}

template <>
struct EnumFlagsTraits<Grindstone::GraphicsAPI::BufferUsage> {
	static constexpr const char* names[] = {
		"Vertex",
		"Index",
		"Uniform",
		"Storage",
		"Indirect",
		"TransferSrc",
		"TransferDst"
	};
	static constexpr size_t size = 7;
};

inline Grindstone::GraphicsAPI::BufferUsage operator|(Grindstone::GraphicsAPI::BufferUsage a, const Grindstone::GraphicsAPI::BufferUsage b) {
	using Underlying = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::BufferUsage>(static_cast<Underlying>(a) | static_cast<Underlying>(b));
}

inline Grindstone::GraphicsAPI::BufferUsage operator&(Grindstone::GraphicsAPI::BufferUsage a, const Grindstone::GraphicsAPI::BufferUsage b) {
	using Underlying = uint8_t;
	return static_cast<Grindstone::GraphicsAPI::BufferUsage>(static_cast<Underlying>(a) & static_cast<Underlying>(b));
}

namespace Grindstone::GraphicsAPI {
	/*! A Buffer is a buffer of memory that exists on the CPU, GPU, or shared
		between them. They can be used by a GraphicsPipeline or ComputePipeline.
	*/
	class Buffer {
	public:
		struct CreateInfo {
			const char* debugName;
			const void* content;
			size_t bufferSize;
			Grindstone::Containers::BitsetFlags<BufferUsage> bufferUsage;
			MemoryUsage memoryUsage;
		};

		Buffer(const Grindstone::GraphicsAPI::Buffer::CreateInfo& createInfo) :
			debugName(createInfo.debugName),
			bufferUsage(createInfo.bufferUsage),
			memoryUsage(createInfo.memoryUsage),
			bufferSize(createInfo.bufferSize) {};

		virtual ~Buffer() {}
		virtual void* Map() = 0;
		virtual void Unmap() = 0;
		virtual void UploadData(const void* data, size_t size, size_t offset = 0) = 0;

		void UploadData(const void* data) {
			UploadData(data, bufferSize, 0);
		}

		BufferUsage GetBufferUsage() const {
			return bufferUsage.GetValueEnum();
		}

		MemoryUsage GetMemoryUsage() const {
			return memoryUsage;
		}

		size_t GetSize() const {
			return bufferSize;
		}

	protected:
		const char* debugName;
		Grindstone::Containers::BitsetFlags<BufferUsage> bufferUsage;
		MemoryUsage memoryUsage;
		size_t bufferSize;
		void* mappedMemoryPtr = nullptr;
	};
};
