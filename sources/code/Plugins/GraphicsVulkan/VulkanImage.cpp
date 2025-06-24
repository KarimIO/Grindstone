#include <assert.h>
#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"
#include "VulkanImage.hpp"

namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::Image::Image(VkImage image, VkFormat format, uint32_t swapchainIndex) :
	imageName(std::string("Image View ") + std::to_string(swapchainIndex)),
	width(1),
	height(1),
	depth(1),
	mipLevels(1),
	arrayLayers(1),
	imageViewType(VkImageViewType::VK_IMAGE_VIEW_TYPE_2D),
	aspect(VK_IMAGE_ASPECT_COLOR_BIT)
{
	UpdateNativeImage(image, imageView, format);
	imageView = CreateImageView(image, imageViewType, vkFormat, aspect, mipLevels, arrayLayers);

	std::string imageViewDebugName = std::string("Image View ") + std::to_string(swapchainIndex);
	Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_IMAGE_VIEW, imageView, imageViewDebugName.c_str());
}

Vulkan::Image::Image(const CreateInfo& createInfo) :
	imageName(createInfo.debugName),
	width(createInfo.width),
	height(createInfo.height),
	depth(createInfo.depth),
	mipLevels(createInfo.mipLevels),
	arrayLayers(createInfo.arrayLayers),
	format(createInfo.format),
	imageUsage(createInfo.imageUsage)
{
	Create();

	bool hasInitialData = createInfo.initialData != nullptr && createInfo.initialDataSize > 0;
	if (hasInitialData) {
		UploadData(createInfo.initialData, createInfo.initialDataSize);

		if (imageUsage.Test(ImageUsageFlags::GenerateMipmaps) && (aspect & VK_IMAGE_ASPECT_COLOR_BIT)) {
			VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
			GenerateMipmaps(commandBuffer, image);
			EndSingleTimeCommands(commandBuffer);
		}
	}

	if (imageUsage.Test(ImageUsageFlags::Sampled)) {
		TransitionImageLayout(
			image,
			vkFormat,
			aspect,
			hasInitialData
				? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				: VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			mipLevels,
			arrayLayers
		);
	}
}

void Vulkan::Image::Create() {
	FormatDepthStencilType type = GetFormatDepthStencilType(format);

	if (imageUsage.Test(ImageUsageFlags::DepthStencil)) {
		// TODO: For renderpasses, a DepthStencil ImageView requires both bits, but for sampled images, the ImageView requires only one.
		// How can we go about fixing this issue?
		switch (type) {
		case FormatDepthStencilType::DepthStencil:
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		case FormatDepthStencilType::StencilOnly:
			aspect = VK_IMAGE_ASPECT_STENCIL_BIT;
			break;
		case FormatDepthStencilType::DepthOnly:
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
			break;
		}
	}
	else {
		aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	uint8_t axisCount =
		((width > 1) ? 1 : 0) +
		((height > 1) ? 1 : 0) +
		((depth > 1) ? 1 : 0);

	if (imageUsage.Test(ImageUsageFlags::Cubemap)) {
		if (arrayLayers > 6) {
			imageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
		}
		else {
			imageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_CUBE;
		}
	}
	else if (arrayLayers > 1) {
		if (axisCount == 1) {
			imageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		}
		else if (axisCount == 2) {
			imageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		}
	}
	else {
		if (axisCount == 1) {
			imageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_1D;
		}
		else if (axisCount == 2) {
			imageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		}
		else if (axisCount == 3) {
			imageViewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_3D;
		}
	}

	this->CreateImage();
	imageView = CreateImageView(image, imageViewType, vkFormat, aspect, mipLevels, arrayLayers);

	if (!imageName.empty()) {
		std::string imageViewDebugName = imageName + " View";
		std::string imageMemoryDebugName = imageName + " Memory";
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_IMAGE, image, imageName.c_str());
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_IMAGE_VIEW, imageView, imageViewDebugName.c_str());
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_DEVICE_MEMORY, imageMemory, imageMemoryDebugName.c_str());
	}
	else {
		GPRINT_WARN(LogSource::GraphicsAPI, "Unnamed image!");
	}
}

void Vulkan::Image::UpdateNativeImage(VkImage image, VkImageView imageView, VkFormat format) {
	this->image = image;
	this->imageView = imageView;
	vkFormat = format;
}

void Vulkan::Image::GenerateMipmaps(VkCommandBuffer commandBuffer, VkImage image) {
	VkPhysicalDevice physicalDevice = Vulkan::Core::Get().GetPhysicalDevice();

	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, vkFormat, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "texture image format does not support linear blitting!");
	}

	int32_t mipWidth = width;
	int32_t mipHeight = height;
	int32_t mipDepth = depth;

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = aspect;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	for (uint32_t mipIndex = 1; mipIndex < mipLevels; ++mipIndex) {
		for (uint32_t arraylayerIndex = 0; arraylayerIndex < arrayLayers; ++arraylayerIndex) {
			barrier.subresourceRange.baseMipLevel = mipIndex - 1;
			barrier.subresourceRange.baseArrayLayer = arraylayerIndex;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, mipDepth };
			blit.srcSubresource.aspectMask = aspect;
			blit.srcSubresource.mipLevel = mipIndex - 1;
			blit.srcSubresource.baseArrayLayer = arraylayerIndex;
			blit.srcSubresource.layerCount = 1;

			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = {
				mipWidth > 1 ? mipWidth >> 1 : 1,
				mipHeight > 1 ? mipHeight >> 1 : 1,
				mipDepth > 1 ? mipDepth >> 1 : 1
			};
			blit.dstSubresource.aspectMask = aspect;
			blit.dstSubresource.mipLevel = mipIndex;
			blit.dstSubresource.baseArrayLayer = arraylayerIndex;
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

			vkCmdPipelineBarrier(
				commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			mipWidth = mipWidth > 1;
			mipHeight /= 2;
			mipHeight /= 2;
		}
	}

	barrier.subresourceRange.aspectMask = aspect;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = arrayLayers;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands(commandBuffer);
}

void Vulkan::Image::Resize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;

	VkDevice device = Vulkan::Core::Get().GetDevice();
	vkDestroyImageView(device, imageView, nullptr);
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
	Create();
}

Vulkan::Image::~Image() {
	VkDevice device = Vulkan::Core::Get().GetDevice();
	vkDestroyImageView(device, imageView, nullptr);

	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
}

void Vulkan::Image::CreateImage() {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	vkFormat = TranslateFormatToVulkan(format);

	VkImageCreateFlags createFlags = 0;
	if (imageUsage.Test(ImageUsageFlags::Cubemap)) {
		createFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	VkImageUsageFlags usageFlags = 0;
	if (imageUsage.Test(ImageUsageFlags::TransferDst)) {
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	if (imageUsage.Test(ImageUsageFlags::TransferSrc)) {
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (imageUsage.Test(ImageUsageFlags::Sampled)) {
		usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	if (imageUsage.Test(ImageUsageFlags::Storage)) {
		usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (imageUsage.Test(ImageUsageFlags::RenderTarget)) {
		usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	if (imageUsage.Test(ImageUsageFlags::DepthStencil)) {
		usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}

	maxImageSize = Vulkan::CreateImage(
		width,
		height,
		depth,
		mipLevels,
		arrayLayers,
		vkFormat,
		VK_IMAGE_TILING_OPTIMAL,
		usageFlags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image,
		imageMemory,
		createFlags
	);

	if (imageUsage.Test(ImageUsageFlags::Storage)) {
		TransitionImageLayout(
			image,
			vkFormat,
			aspect,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			mipLevels,
			arrayLayers
		);
	}
}

void Vulkan::Image::UploadData(const char* data, uint64_t dataSize) {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(
		imageName.c_str(),
		dataSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	void* mappedData;
	vkMapMemory(device, stagingBufferMemory, 0, dataSize, 0, &mappedData);
	memcpy(mappedData, data, static_cast<size_t>(dataSize));
	vkUnmapMemory(device, stagingBufferMemory);

	TransitionImageLayout(
		image,
		vkFormat,
		aspect,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		mipLevels,
		arrayLayers
	);

	bool isCompressedFormat = IsFormatCompressed(format);
	uint64_t blockSize = GetCompressedFormatBlockSize(format);
	uint64_t pixelSize = GetFormatBytesPerPixel(format);

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	uint64_t offset = 0;

	std::vector<VkBufferImageCopy> regions;
	regions.reserve(static_cast<size_t>(mipLevels) * static_cast<size_t>(arrayLayers));

	for (uint32_t layerIndex = 0; layerIndex < arrayLayers; ++layerIndex) {
		uint64_t mipWidth = width;
		uint64_t mipHeight = height;

		for (uint32_t mipIndex = 0; mipIndex < mipLevels; ++mipIndex) {
			VkBufferImageCopy& region = regions.emplace_back();
			region.bufferOffset = offset;
			region.bufferRowLength = 0;
			region.imageSubresource.baseArrayLayer = layerIndex;
			region.imageSubresource.aspectMask = aspect;
			region.imageSubresource.mipLevel = mipIndex;
			region.imageSubresource.layerCount = 1;
			region.imageExtent.width = static_cast<uint32_t>(mipWidth);
			region.imageExtent.height = static_cast<uint32_t>(mipHeight);
			region.imageExtent.depth = 1;

			uint64_t mipSize = isCompressedFormat
				? ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize
				: mipWidth * mipHeight * pixelSize;


			offset += mipSize;

			mipWidth = mipWidth >> 1;
			if (mipWidth < 1) {
				mipWidth = 1;
			}

			mipHeight = mipHeight >> 1;
			if (mipHeight < 1) {
				mipHeight = 1;
			}
		}
	}

	GS_ASSERT_ENGINE(offset <= dataSize);

	vkCmdCopyBufferToImage(
		commandBuffer,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(regions.size()),
		regions.data()
	);

	EndSingleTimeCommands(commandBuffer);
	
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

VkImage Vulkan::Image::GetImage() const {
	return image;
}

VkImageView Vulkan::Image::GetImageView() const {
	return imageView;
}

uint32_t Vulkan::Image::GetWidth() const {
	return width;
}

uint32_t Vulkan::Image::GetHeight() const {
	return height;
}

uint32_t Vulkan::Image::GetDepth() const {
	return depth;
}

uint32_t Vulkan::Image::GetMipLevels() const {
	return mipLevels;
}

uint32_t Vulkan::Image::GetArrayLayers() const {
	return arrayLayers;
}

VkImageAspectFlags Vulkan::Image::GetAspect() const {
	return aspect;
}
