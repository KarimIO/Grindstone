#pragma once

#include "../GraphicsCommon/GraphicsPipeline.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanGraphicsPipeline : public GraphicsPipeline {
		public:
			VulkanGraphicsPipeline(GraphicsPipelineCreateInfo ci);
			virtual ~VulkanGraphicsPipeline() override;
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