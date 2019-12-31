#pragma once

#include "../GraphicsCommon/RenderPass.hpp"
#include <vulkan/vulkan.h>

class VulkanRenderPass : public RenderPass {
public:
	VkRenderPass renderPass;
	VkDevice *device;

public:
	uint32_t m_width;
	uint32_t m_height;
	ClearColorValue *m_colorClearValues;
	uint32_t m_colorClearCount;
	ClearDepthStencil m_depthStencilClearValue;

	VulkanRenderPass(VkDevice *, VkFormat, RenderPassCreateInfo);
	~VulkanRenderPass();

	VkRenderPass *GetRenderPass();
};