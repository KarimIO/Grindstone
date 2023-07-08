#include <assert.h>
#include "VulkanTexture.hpp"
#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"
#include <vulkan/vulkan.h>

using namespace Grindstone::GraphicsAPI;

VulkanTexture::VulkanTexture(Texture::CreateInfo& createInfo) {
	uint32_t mipLevels;
	CreateTextureImage(createInfo, mipLevels);
	imageView = CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	CreateTextureSampler(createInfo, mipLevels);

	std::string debugName = createInfo.debugName;
	std::string imageViewDebugName = debugName + " View";
	std::string imageSamplerDebugName = debugName + " Sampler";
	VulkanCore::Get().NameObject(VK_OBJECT_TYPE_IMAGE, image, createInfo.debugName);
	VulkanCore::Get().NameObject(VK_OBJECT_TYPE_IMAGE_VIEW, imageView, imageViewDebugName.c_str());
	VulkanCore::Get().NameObject(VK_OBJECT_TYPE_SAMPLER, sampler, imageSamplerDebugName.c_str());
}

void VulkanTexture::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
	VkPhysicalDevice physicalDevice = VulkanCore::Get().GetPhysicalDevice();

	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	EndSingleTimeCommands(commandBuffer);
}

VulkanTexture::~VulkanTexture() {
	VkDevice device = VulkanCore::Get().GetDevice();
	vkDestroySampler(device, sampler, nullptr);
	vkDestroyImageView(device, imageView, nullptr);

	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
}

void VulkanTexture::CreateTextureImage(Texture::CreateInfo& createInfo, uint32_t& mipLevels) {
	VkDevice device = VulkanCore::Get().GetDevice();

	uint8_t channels = 4;
	format = TranslateColorFormatToVulkan(createInfo.format, channels);

	bool isSmallCompressedFormat =
		createInfo.format == ColorFormat::SRGB_DXT1 ||
		createInfo.format == ColorFormat::SRGB_ALPHA_DXT1 ||
		createInfo.format == ColorFormat::RGB_DXT1 ||
		createInfo.format == ColorFormat::RGBA_DXT1 ||
		createInfo.format == ColorFormat::BC4;
	bool isLargeCompressedFormat =
		createInfo.format == ColorFormat::SRGB_ALPHA_DXT3 ||
		createInfo.format == ColorFormat::SRGB_ALPHA_DXT5 ||
		createInfo.format == ColorFormat::RGBA_DXT3 ||
		createInfo.format == ColorFormat::RGBA_DXT5;
	bool isCompressedFormat = isSmallCompressedFormat || isLargeCompressedFormat;

	uint32_t blockSize = (isSmallCompressedFormat) ? 8 : 16;

	uint32_t baseMipSize = isCompressedFormat
		? ((createInfo.width + 3) / 4) * ((createInfo.height + 3) / 4) * blockSize
		: createInfo.width * createInfo.height * channels;
	uint32_t totalImageSize = 0;

	bool shouldGenerateMipmaps = createInfo.options.shouldGenerateMipmaps;
	if (shouldGenerateMipmaps) {
		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(createInfo.width, createInfo.height)))) + 1;

		// If we're DXT, we can't generate mipmaps
		if (isCompressedFormat) {
			shouldGenerateMipmaps = false;
			mipLevels = 1;
		}
	}
	else {
		mipLevels = (createInfo.mipmaps > 2) ? (createInfo.mipmaps - 2) : 1;

		uint32_t width = createInfo.width;
		uint32_t height = createInfo.height;
		for (uint32_t i = 0; i < mipLevels; ++i) {
			uint32_t mipSize = isCompressedFormat
				? ((width + 3) / 4) * ((width + 3) / 4) * blockSize
				: height * height * channels;
			width /= 2;
			height /= 2;
			totalImageSize += mipSize;
		}
	}

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(
		createInfo.debugName,
		totalImageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, totalImageSize, 0, &data);
	memcpy(data, createInfo.data, static_cast<size_t>(totalImageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	CreateImage(
		createInfo.width,
		createInfo.height,
		mipLevels,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image,
		imageMemory
	);

	TransitionImageLayout(image, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

	if (shouldGenerateMipmaps) {
		CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(createInfo.width), static_cast<uint32_t>(createInfo.height));
	}
	else {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
		uint32_t offset = 0;
		uint32_t mipWidth = createInfo.width;
		uint32_t mipHeight = createInfo.height;

		std::vector<VkBufferImageCopy> regions;
		regions.resize(mipLevels);
		for (uint32_t i = 0; i < mipLevels; ++i) {
			VkBufferImageCopy& region = regions[i];
			region.bufferOffset = offset;
			region.bufferRowLength = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = i;
			region.imageSubresource.layerCount = 1;
			region.imageExtent.width = mipWidth;
			region.imageExtent.height = mipHeight;
			region.imageExtent.depth = 1;

			uint32_t mipSize = isCompressedFormat
				? ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize
				: mipWidth * mipHeight * channels;
			offset += mipSize;
			mipWidth /= 2;
			mipHeight /= 2;
		}

		vkCmdCopyBufferToImage(
			commandBuffer,
			stagingBuffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(regions.size()),
			regions.data()
		);

		EndSingleTimeCommands(commandBuffer);
		TransitionImageLayout(image, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);
	}
	
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	if (shouldGenerateMipmaps) {
		GenerateMipmaps(image, format, createInfo.width, createInfo.height, mipLevels);
	}
}

void VulkanTexture::CreateTextureSampler(Texture::CreateInfo &createInfo, uint32_t mipLevels) {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = TranslateFilterToVulkan(createInfo.options.magFilter);
	samplerInfo.minFilter = TranslateFilterToVulkan(createInfo.options.minFilter);
	samplerInfo.addressModeU = TranslateWrapToVulkan(createInfo.options.wrapModeU);
	samplerInfo.addressModeV = TranslateWrapToVulkan(createInfo.options.wrapModeV);
	samplerInfo.addressModeW = TranslateWrapToVulkan(createInfo.options.wrapModeW);
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0;
	samplerInfo.maxLod = static_cast<float>(mipLevels);
	samplerInfo.mipLodBias = 0;

	if (vkCreateSampler(VulkanCore::Get().GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void VulkanTexture::RecreateTexture(CreateInfo& createInfo) {
	std::cout << "VulkanTexture::RecreateTexture is not used\n";
	assert(false);
}

VkImageView VulkanTexture::GetImageView() {
	return imageView;
}

VkSampler VulkanTexture::GetSampler() {
	return sampler;
}
