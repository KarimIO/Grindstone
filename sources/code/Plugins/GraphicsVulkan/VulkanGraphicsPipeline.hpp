#pragma once

#include <Common/Graphics/GraphicsPipeline.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanGraphicsPipeline : public GraphicsPipeline {
		public:
			VulkanGraphicsPipeline(GraphicsPipeline::CreateInfo& createInfo);
			~VulkanGraphicsPipeline();
			VkPipeline GetGraphicsPipeline();
			VkPipelineLayout GetGraphicsPipelineLayout();
		public:
			virtual void Bind() {};
			virtual void Recreate(GraphicsPipeline::CreateInfo& createInfo) override;
		private:
			void CreateShaderModule(ShaderStageCreateInfo &createInfo, VkPipelineShaderStageCreateInfo &out);
		private:
			VkPipelineLayout pipelineLayout = nullptr;
			VkPipeline graphicsPipeline = nullptr;
		};
	}
}
