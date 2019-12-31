#pragma once

#include <vulkan/vulkan.h>
#include "../GraphicsCommon/GraphicsPipeline.hpp"

class VulkanGraphicsPipeline : public GraphicsPipeline {
	VkDevice *device;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;

	VkShaderModule createShaderModule(ShaderStageCreateInfo shaderStageCreateInfo);
public:
	VulkanGraphicsPipeline(VkDevice *dev, GraphicsPipelineCreateInfo createInfo);
	VkPipeline *GetGraphicsPipeline();
	VkPipelineLayout *GetPipelineLayout();
	~VulkanGraphicsPipeline();
};