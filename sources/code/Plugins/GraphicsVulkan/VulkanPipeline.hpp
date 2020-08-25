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
			virtual ~VulkanPipeline() override;
			VkPipeline getGraphicsPipeline();
			VkPipelineLayout getGraphicsPipelineLayout();
		public:
			virtual void Bind() {};
		private:
			void createShaderModule(ShaderStageCreateInfo &ci, VkPipelineShaderStageCreateInfo &out);
		private:
			VkPipelineLayout pipeline_layout_;
			VkPipeline graphics_pipeline_;
		};
	}
}