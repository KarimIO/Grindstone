#pragma once

#include <vulkan/vulkan.h>
#include "../GraphicsCommon/GraphicsPipeline.h"

class vkGraphicsPipeline : public GraphicsPipeline {
	VkDevice *device;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;

	VkShaderModule createShaderModule(ShaderStageCreateInfo shaderStageCreateInfo);
public:
	vkGraphicsPipeline(VkDevice *dev, GraphicsPipelineCreateInfo createInfo);
	VkPipeline *GetGraphicsPipeline();
	VkPipelineLayout *GetPipelineLayout();
	~vkGraphicsPipeline();
};