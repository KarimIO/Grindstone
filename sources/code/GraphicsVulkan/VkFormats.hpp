#pragma once

#include "../GraphicsCommon/Formats.hpp"
#include <vulkan/vulkan.h>

VkFormat TranslateDepthFormat(DepthFormat format);

VkCommandBuffer BeginSingleTimeCommandBuffer(VkDevice *device, VkCommandPool *);
void EndSingleTimeCommandBuffer(VkDevice *device, VkCommandPool *commandPool, VkQueue *graphicsQueue, VkCommandBuffer commandBuffer);