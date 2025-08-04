#pragma once

#include "../GraphicsCommon/VertexBuffer.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12VertexBuffer : public VertexBuffer {
		public:
			DirectX12VertexBuffer(VertexBufferCreateInfo ci);
			virtual ~DirectX12VertexBuffer() override;
		public:
			//VkBuffer getBuffer();
		private:
			ID3D12Resource* vertex_buffer_;
			D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
		};
	};
};