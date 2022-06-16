#pragma once

#include <Common/Graphics/Pipeline.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanPipeline : public Pipeline {
		public:
			VulkanPipeline(Pipeline::CreateInfo& ci);
			~VulkanPipeline();
			VkPipeline GetGraphicsPipeline();
			VkPipelineLayout GetGraphicsPipelineLayout();
		public:
			virtual void Bind() {};
			virtual void Recreate(CreateInfo& createInfo) override;
		private:
			void CreateShaderModule(ShaderStageCreateInfo &ci, VkPipelineShaderStageCreateInfo &out);
		private:
			VkPipelineLayout pipelineLayout;
			VkPipeline graphicsPipeline;
		};
	}
}
