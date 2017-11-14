#pragma once

#include "../GraphicsCommon/RenderPass.h"
#include <vulkan/vulkan.h>

class vkRenderPass : public RenderPass {
public:
	VkRenderPass renderPass;
	VkDevice *device;

public:
	uint32_t m_width;
	uint32_t m_height;
	ClearColorValue *m_colorClearValues;
	uint32_t m_colorClearCount;
	ClearDepthStencil m_depthStencilClearValue;

	vkRenderPass(VkDevice *, VkFormat, RenderPassCreateInfo);
	~vkRenderPass();

	VkRenderPass *GetRenderPass();
};