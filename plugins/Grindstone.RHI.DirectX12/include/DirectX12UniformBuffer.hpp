#pragma once

#include "../GraphicsCommon/UniformBuffer.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class DirectX12UniformBufferBinding : public UniformBufferBinding {
		public:
			DirectX12UniformBufferBinding(UniformBufferBindingCreateInfo ci);
			virtual ~DirectX12UniformBufferBinding();
		public:
			//VkDescriptorSetLayout getDescriptorSetLayout();
		private:
			//VkDescriptorSetLayout descriptor_set_layout_;
		};

		class DirectX12UniformBuffer : public UniformBuffer {
		public:
			DirectX12UniformBuffer(UniformBufferCreateInfo ci);
			virtual ~DirectX12UniformBuffer();
		public:
			//VkDescriptorSet getDescriptorSet();
		public:
			virtual void UpdateUniformBuffer(void * content) override;
			virtual void Bind() override;
		private:
			//VkDescriptorSet descriptor_set_;
			//VkBuffer buffer_;
			//VkDeviceMemory memory_;
			uint32_t size_;
		};
	};
};
