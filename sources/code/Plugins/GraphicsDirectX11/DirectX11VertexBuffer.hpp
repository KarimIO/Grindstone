#pragma once

#include "../GraphicsCommon/VertexBuffer.hpp"
#include <d3d11.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11VertexBuffer : public VertexBuffer {
		public:
			DirectX11VertexBuffer(VertexBufferCreateInfo ci);
			virtual ~DirectX11VertexBuffer() override;
		public:
			void Bind();
		private:
			ID3D11Buffer* buffer_;
			UINT stride_;
		};
	};
};