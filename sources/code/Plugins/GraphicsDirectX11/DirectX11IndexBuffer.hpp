#pragma once

#include "../GraphicsCommon/IndexBuffer.hpp"
#include <d3d11.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11IndexBuffer : public IndexBuffer {
		public:
			DirectX11IndexBuffer(IndexBufferCreateInfo ci);
			virtual ~DirectX11IndexBuffer() override;
		public:
			void Bind();
		private:
			ID3D11Buffer* index_buffer_;
		};
	}
}