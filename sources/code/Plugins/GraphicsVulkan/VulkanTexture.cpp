#include <assert.h>
#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"
#include "VulkanTexture.hpp"

using namespace Grindstone::GraphicsAPI;

VulkanTexture::VulkanTexture(Texture::CreateInfo& createInfo) {
	if (createInfo.debugName == nullptr) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Texture!");
	}

	uint32_t mipLevels;
	CreateTextureImage(createInfo, mipLevels, createInfo.isCubemap ? 6 : 1);
	imageView = createInfo.isCubemap
		? CreateCubemapView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels)
		: CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
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
		GPRINT_FATAL(LogSource::GraphicsAPI, "texture image format does not support linear blitting!");
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

void VulkanTexture::CreateTextureImage(Texture::CreateInfo& createInfo, uint32_t& mipLevels, uint32_t layerCount) {
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
		createInfo.format == ColorFormat::RGBA_DXT5 ||
		createInfo.format == ColorFormat::BC6H;
	bool isCompressedFormat = isSmallCompressedFormat || isLargeCompressedFormat;

	uint64_t blockSize = (isSmallCompressedFormat) ? 8 : 16;

	if (!isCompressedFormat) {
		switch (createInfo.format) {
			case ColorFormat::R16:
			case ColorFormat::RG16:
			case ColorFormat::RGB16:
			case ColorFormat::RGBA16:
				channels *= 2;
				break;
			case ColorFormat::R32:
			case ColorFormat::RG32:
			case ColorFormat::RGB32:
			case ColorFormat::RGBA32:
				channels *= 4;
				break;
		}
	}

	uint64_t baseMipSize = isCompressedFormat
		? ((createInfo.width + 3) / 4) * ((createInfo.height + 3) / 4) * blockSize
		: createInfo.width * createInfo.height * channels;
	uint64_t totalImageSize = 0;

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
		uint32_t maxMipMaps = isCompressedFormat
			? static_cast<uint32_t>(std::floor(std::log2(std::max(createInfo.width, createInfo.height)))) - 1
			: static_cast<uint32_t>(std::floor(std::log2(std::max(createInfo.width, createInfo.height)))) + 1;
		mipLevels = (createInfo.mipmaps > maxMipMaps) ? maxMipMaps : createInfo.mipmaps;

		uint64_t width = createInfo.width;
		uint64_t height = createInfo.height;
		for (uint32_t i = 0; i < mipLevels; ++i) {
			uint64_t mipSize = isCompressedFormat
				? ((width + 3) / 4) * ((height + 3) / 4) * blockSize
				: width * height * channels;
			width = width >> 1;
			height = height >> 1;
			totalImageSize += mipSize;
		}
	}

	totalImageSize *= layerCount;

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
		imageMemory,
		layerCount,
		layerCount == 6 ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0
	);

	TransitionImageLayout(image, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, layerCount);

	if (shouldGenerateMipmaps) {
		CopyBufferToImage(stagingBuffer, image, static_cast<uint64_t>(createInfo.width), static_cast<uint64_t>(createInfo.height));
	}
	else {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
		uint64_t offset = 0;
		for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex) {
			uint64_t mipWidth = createInfo.width;
			uint64_t mipHeight = createInfo.height;

			std::vector<VkBufferImageCopy> regions;
			regions.resize(mipLevels);
			for (uint32_t mipIndex = 0; mipIndex < mipLevels; ++mipIndex) {
				VkBufferImageCopy& region = regions[mipIndex];
				region.bufferOffset = offset;
				region.bufferRowLength = 0;
				region.imageSubresource.baseArrayLayer = layerIndex;
				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.mipLevel = mipIndex;
				region.imageSubresource.layerCount = 1;
				region.imageExtent.width = static_cast<uint32_t>(mipWidth);
				region.imageExtent.height = static_cast<uint32_t>(mipHeight);
				region.imageExtent.depth = 1;

				uint64_t mipSize = isCompressedFormat
					? ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize
					: mipWidth * mipHeight * channels;
				offset += mipSize;
				mipWidth = mipWidth >> 1;
				mipHeight = mipHeight >> 1;
			}

			vkCmdCopyBufferToImage(
				commandBuffer,
				stagingBuffer,
				image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				static_cast<uint32_t>(regions.size()),
				regions.data()
			);
		}

		EndSingleTimeCommands(commandBuffer);
		TransitionImageLayout(image, format, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels, layerCount);
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
	samplerInfo.anisotropyEnable = (createInfo.options.anistropy != 0) ? VK_TRUE : VK_FALSE;

	// TODO: anisotropy must be between 1 and device limit
	samplerInfo.maxAnisotropy = createInfo.options.anistropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.mipmapMode = TranslateMipFilterToVulkan(createInfo.options.mipFilter);
	samplerInfo.minLod = createInfo.options.mipMin;
	samplerInfo.maxLod = createInfo.options.mipMax;
	samplerInfo.mipLodBias = createInfo.options.mipBias;

	if (vkCreateSampler(VulkanCore::Get().GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create texture sampler!");
	}
}

void VulkanTexture::RecreateTexture(CreateInfo& createInfo) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanTexture::RecreateTexture is not used");
}

VkImageView VulkanTexture::GetImageView() const {
	return imageView;
}

VkSampler VulkanTexture::GetSampler() const {
	return sampler;
}
