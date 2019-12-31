#include "VkFormats.hpp"

VkFormat TranslateDepthFormat(DepthFormat format) {
	switch (format) {
	case FORMAT_DEPTH_16:
		return VK_FORMAT_D16_UNORM;
	case FORMAT_DEPTH_24:
		return VK_FORMAT_X8_D24_UNORM_PACK32;
	default:
	case FORMAT_DEPTH_32:
		return VK_FORMAT_D32_SFLOAT;
	//case FORMAT_DEPTH_16_STENCIL_8:
	//	return VK_FORMAT_D16_UNORM_S8_UINT;
	case FORMAT_DEPTH_24_STENCIL_8:
		return VK_FORMAT_D24_UNORM_S8_UINT;
	case FORMAT_DEPTH_32_STENCIL_8:
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	//case FORMAT_STENCIL_8:
	//	return VK_FORMAT_S8_UINT;
	}
}

VkCommandBuffer BeginSingleTimeCommandBuffer(VkDevice *device, VkCommandPool *commandPool) {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = *commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(*device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void EndSingleTimeCommandBuffer(VkDevice *device, VkCommandPool *commandPool, VkQueue *graphicsQueue, VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(*graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(*graphicsQueue);

	vkFreeCommandBuffers(*device, *commandPool, 1, &commandBuffer);
}