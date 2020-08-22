#include "DirectX11VertexBuffer.hpp"
#include "DirectX11GraphicsWrapper.hpp"
#include "DirectX11Utils.hpp"
#include "DirectX11VertexArrayObject.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX11VertexArrayObject::DirectX11VertexArrayObject(VertexArrayObjectCreateInfo ci) {
			num_vbo = ci.vertex_buffer_count;

			vbos = new DirectX11VertexBuffer*[num_vbo];
			for (uint32_t i = 0; i < num_vbo; ++i) {
				vbos[i] = (DirectX11VertexBuffer *)ci.vertex_buffers[i];
			}

			ibo = (DirectX11IndexBuffer*)ci.index_buffer;
		}
		
		DirectX11VertexArrayObject::~DirectX11VertexArrayObject() {
		}

		void DirectX11VertexArrayObject::Bind() {
			for (uint32_t i = 0; i < num_vbo; ++i) {
				vbos[i]->Bind();
			}

			if (ibo)
				ibo->Bind();
		}
		
		void DirectX11VertexArrayObject::Unbind()
		{
		}
	}
}