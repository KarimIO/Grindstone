#pragma once

#include "../GraphicsCommon/VertexArrayObject.hpp"
#include "DirectX11VertexBuffer.hpp"
#include "DirectX11IndexBuffer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11VertexArrayObject : public VertexArrayObject {
		public:
			DirectX11VertexArrayObject(VertexArrayObjectCreateInfo ci);
			virtual ~DirectX11VertexArrayObject() override;
			virtual void Bind() override;
			virtual void Unbind() override;
		private:
			uint32_t num_vbo;
			DirectX11VertexBuffer** vbos;
			DirectX11IndexBuffer* ibo;
		};
	};
};
