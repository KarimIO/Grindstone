#pragma once

#include "../GraphicsCommon/IndexBuffer.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12IndexBuffer : public IndexBuffer {
		public:
			DirectX12IndexBuffer(IndexBufferCreateInfo ci);
			virtual ~DirectX12IndexBuffer() {};
		public:
			//VkBuffer getBuffer();
		private:
			//VkBuffer buffer_;
			//VkDeviceMemory memory_;
		};
	}
}