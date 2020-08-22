#include "DirectX11IndexBuffer.hpp"
#include "DirectX11GraphicsWrapper.hpp"
#include "DirectX11Utils.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX11IndexBuffer::DirectX11IndexBuffer(IndexBufferCreateInfo ci) {
			D3D11_BUFFER_DESC indexBufferDesc;
			ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			indexBufferDesc.ByteWidth = sizeof(DWORD) * 2 * 3;
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA iinitData;
			iinitData.pSysMem = ci.content;
			DirectX11GraphicsWrapper::get().device_->CreateBuffer(&indexBufferDesc, &iinitData, &index_buffer_);
		}

		DirectX11IndexBuffer::~DirectX11IndexBuffer() {

		}

		void DirectX11IndexBuffer::Bind() {
			DirectX11GraphicsWrapper::get().device_context_->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R16_UINT, 0);
		}
	}
}

