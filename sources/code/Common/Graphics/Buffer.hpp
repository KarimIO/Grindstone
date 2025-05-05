#pragma once

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	enum class BufferUsage : uint8_t {
		Vertex,
		Index,
		Uniform,
		Storage,
		Indirect,
		TransferSrc,
		TransferDst
	};

	enum class MemUsage : uint8_t {
		GPUOnly,
		CPUOnly,
		CPUToGPU,
		GPUToCPU
	};

	/*! A Buffer is a buffer of memory that exists on the CPU, GPU, or shared
		between them. They can be used by a GraphicsPipeline or ComputePipeline.
	*/
	class Buffer {
	public:
		struct CreateInfo {
			const char* debugName;
			size_t bufferSize;
			BufferUsage bufferUsage;
			MemUsage memoryUsage;
		};

		Buffer(const Grindstone::GraphicsAPI::Buffer::CreateInfo& createInfo) :
			debugName(createInfo.debugName),
			bufferUsage(createInfo.bufferUsage),
			memoryUsage(createInfo.memoryUsage),
			bufferSize(createInfo.bufferSize) {};

		virtual ~Buffer() = 0;
		virtual void* Map() = 0;
		virtual void Unmap() = 0;
		virtual void UploadData(const void* data, size_t size, size_t offset = 0) = 0;

		void UploadData(const void* data) {
			UploadData(data, bufferSize, 0);
		}

		BufferUsage GetBufferUsage() const {
			return bufferUsage;
		}

		MemUsage GetMemoryUsage() const {
			return memoryUsage;
		}

		size_t GetSize() const {
			return bufferSize;
		}

	protected:
		const char* debugName;
		BufferUsage bufferUsage;
		MemUsage memoryUsage;
		size_t bufferSize;
		void* mappedMemoryPtr = nullptr;
	};
};
