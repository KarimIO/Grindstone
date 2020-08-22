#include "DirectX11VertexBuffer.hpp"
#include "DirectX11GraphicsWrapper.hpp"
#include "DirectX11Utils.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX11VertexBuffer::DirectX11VertexBuffer(VertexBufferCreateInfo ci) {
			auto device = DirectX11GraphicsWrapper::get().device_;
			auto device_context = DirectX11GraphicsWrapper::get().device_context_;

			stride_ = ci.layout->stride;
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));

			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = ci.size;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			device->CreateBuffer(&bd, NULL, &buffer_);       // create the buffer

			D3D11_MAPPED_SUBRESOURCE ms;
			device_context->Map(buffer_, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);	// map the buffer
			memcpy(ms.pData, ci.content, ci.size);										// copy the data
			device_context->Unmap(buffer_, NULL);										// unmap the buffer
		}

		DirectX11VertexBuffer::~DirectX11VertexBuffer() {
			buffer_->Release();
		}

		void DirectX11VertexBuffer::Bind() {
			UINT offset = 0;
			DirectX11GraphicsWrapper::get().device_context_->IASetVertexBuffers(0, 1, &buffer_, &stride_, &offset);
		}
	}
}