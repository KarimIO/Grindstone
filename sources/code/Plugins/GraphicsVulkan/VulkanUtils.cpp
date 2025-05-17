#include <EngineCore/Logger.hpp>

#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"

namespace Grindstone::GraphicsAPI::Vulkan {
	VkImageView CreateImageView(VkImage image, VkImageViewType imageViewType, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, uint32_t arrayLayers) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = imageViewType;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = arrayLayers;

		VkImageView imageView;
		if (vkCreateImageView(Vulkan::Core::Get().GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create texture image view!");
		}

		return imageView;
	}
		
	VkImageView CreateCubemapView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;

		VkImageView imageView;
		if (vkCreateImageView(Vulkan::Core::Get().GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create texture image view!");
		}

		return imageView;
	}

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
	) {
		VkDevice device = Vulkan::Core::Get().GetDevice();
			
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = depth;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = layerCount;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = flags;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Vulkan::Core::Get().FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);

		return memRequirements.size;
	}

	void CreateBuffer(
		const char* debugName,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory
	) {
		if (debugName == nullptr) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Buffer!");
		}

		VkDevice device = Vulkan::Core::Get().GetDevice();
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create buffer!");
		}

		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_BUFFER, buffer, debugName);

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Vulkan::Core::Get().FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to allocate buffer memory!");
		}

		std::string memoryName = std::string(debugName) + " Memory";
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_DEVICE_MEMORY, bufferMemory, memoryName.c_str());

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	VkCommandBuffer BeginSingleTimeCommands() {
		VkDevice device = Vulkan::Core::Get().GetDevice();
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = Vulkan::Core::Get().GetGraphicsCommandPool();
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
		VkDevice device = Vulkan::Core::Get().GetDevice();
		auto graphicsQueue = Vulkan::Core::Get().graphicsQueue;
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, Vulkan::Core::Get().GetGraphicsCommandPool(), 1, &commandBuffer);
	}

	VkShaderStageFlags TranslateShaderStageBits(ShaderStageBit shaderStageBits) {
		VkShaderStageFlags outputStages = 0;

		uint8_t bit = static_cast<uint8_t>(shaderStageBits);
		if (bit & static_cast<uint8_t>(ShaderStageBit::Vertex)) {
			outputStages |= VK_SHADER_STAGE_VERTEX_BIT;
		}

		if (bit & static_cast<uint8_t>(ShaderStageBit::Fragment)) {
			outputStages |= VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		if (bit & static_cast<uint8_t>(ShaderStageBit::Compute)) {
			outputStages |= VK_SHADER_STAGE_COMPUTE_BIT;
		}

		if (bit & static_cast<uint8_t>(ShaderStageBit::Geometry)) {
			outputStages |= VK_SHADER_STAGE_GEOMETRY_BIT;
		}

		if (bit & static_cast<uint8_t>(ShaderStageBit::Task)) {
			outputStages |= VK_SHADER_STAGE_TASK_BIT_EXT;
		}

		if (bit & static_cast<uint8_t>(ShaderStageBit::Mesh)) {
			outputStages |= VK_SHADER_STAGE_MESH_BIT_EXT;
		}

		if (bit & static_cast<uint8_t>(ShaderStageBit::TesselationControl)) {
			outputStages |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		}

		if (bit & static_cast<uint8_t>(ShaderStageBit::TesselationEvaluation)) {
			outputStages |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		}

		return outputStages;
	}

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, uint32_t layerCount) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layerCount;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_GENERAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL || newLayout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL)) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage,
			destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommands(commandBuffer);
	}

	void CopyBufferToImage(
		VkBuffer buffer,
		VkImage image,
		VkImageAspectFlags aspect,
		uint32_t width,
		uint32_t height,
		uint32_t depth,
		[[maybe_unused]] uint32_t mipLevels,
		uint32_t arrayLayers
	) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = aspect;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = arrayLayers;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			depth
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		EndSingleTimeCommands(commandBuffer);
	}
}
