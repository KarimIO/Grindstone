#include "DirectX11UniformBuffer.hpp"
#include "DirectX11GraphicsWrapper.hpp"
#include "DirectX11Utils.hpp"
#include <iostream>

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX11UniformBufferBinding::DirectX11UniformBufferBinding(UniformBufferBindingCreateInfo ci) {
			stages_ = ci.stages;
		}

		DirectX11UniformBufferBinding::~DirectX11UniformBufferBinding() {
		}


		//==========================


		DirectX11UniformBuffer::DirectX11UniformBuffer(UniformBufferCreateInfo ci) : size_(ci.size) {
			stages_ = ((DirectX11UniformBufferBinding*)ci.binding)->stages_;

			D3D11_BUFFER_DESC bd = { 0 };
			bd.ByteWidth = size_;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;

			float col[] = { 0.4f, 0.7f, 1.0f, 1.0f };
			D3D11_SUBRESOURCE_DATA iinitData;
			iinitData.pSysMem = col;
			iinitData.SysMemPitch = 0;
			iinitData.SysMemSlicePitch = 0;

			if (FAILED(DirectX11GraphicsWrapper::get().device_->CreateBuffer(&bd, &iinitData, &buffer_)))
				throw std::runtime_error("Critical error: Unable to create the constant colour buffer!");
		}

		DirectX11UniformBuffer::~DirectX11UniformBuffer() {
			buffer_->Release();
		}

		void DirectX11UniformBuffer::UpdateUniformBuffer(void * content) {
			/*float col[] = { 0.0f, 0.0f, 0.0f, 1.0f };
			DirectX11GraphicsWrapper::get().device_context_->UpdateSubresource(buffer_, 0, 0, col, 0, 0);*/

			D3D11_MAPPED_SUBRESOURCE mapped_resource; 
			DirectX11GraphicsWrapper::get().device_context_->Map(buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
			//  Update the vertex buffer here.
			memcpy(mapped_resource.pData, content, size_);
			//  Reenable GPU access to the vertex buffer data.
			DirectX11GraphicsWrapper::get().device_context_->Unmap(buffer_, 0);
		};
		
		void DirectX11UniformBuffer::Bind() {
			if ((uint8_t)stages_ & (uint8_t)Grindstone::GraphicsAPI::ShaderStageBit::Vertex) {
				DirectX11GraphicsWrapper::get().device_context_->VSSetConstantBuffers(0, 1, &buffer_);
			}
			if ((uint8_t)stages_ & (uint8_t)Grindstone::GraphicsAPI::ShaderStageBit::Fragment) {
				DirectX11GraphicsWrapper::get().device_context_->PSSetConstantBuffers(0, 1, &buffer_);
			}
		};
		
	};
};
