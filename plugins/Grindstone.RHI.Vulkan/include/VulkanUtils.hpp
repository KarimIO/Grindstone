#pragma once

#include <Common/Graphics/Formats.hpp>
#include <vulkan/vulkan.h>
#include <stdint.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	VkImageView CreateImageView(VkImage image, VkImageViewType imageViewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t arrayLayers);
	VkImageView CreateCubemapView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	VkDeviceSize CreateImage(
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		uint32_t mipLevels,
		uint32_t layerCount,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory,
		VkImageCreateFlags flags
	);
	void CreateBuffer(const char* debugName, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	VkShaderStageFlags TranslateShaderStageBits(ShaderStageBit shaderStageBits);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount);
	void CopyBufferToImage(
		VkBuffer buffer,
		VkImage image,
		VkImageAspectFlags aspect,
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		uint32_t mipLevels,
		uint32_t arrayLayers
	);
}
