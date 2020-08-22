#pragma once

#include "../GraphicsCommon/UniformBuffer.hpp"
#include <d3d11.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX11UniformBufferBinding : public UniformBufferBinding {
		public:
			DirectX11UniformBufferBinding(UniformBufferBindingCreateInfo ci);
			virtual ~DirectX11UniformBufferBinding();
			ShaderStageBit stages_;
		private:
		};

		class DirectX11UniformBuffer : public UniformBuffer {
		public:
			DirectX11UniformBuffer(UniformBufferCreateInfo ci);
			virtual ~DirectX11UniformBuffer();
		public:
			virtual void UpdateUniformBuffer(void * content) override;
			virtual void Bind() override;
		private:
			ShaderStageBit stages_;
			ID3D11Buffer* buffer_;
			uint32_t size_;
		};
	};
};
