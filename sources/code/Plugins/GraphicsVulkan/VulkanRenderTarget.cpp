#include "VulkanRenderTarget.hpp"
#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"
#include <iostream>
#include <cassert>

using namespace Grindstone::GraphicsAPI;

VulkanRenderTarget::VulkanRenderTarget(VkImage swapchainImage, VkFormat format) : image(swapchainImage), isOwnedBySwapchain(true) {
	imageView = CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

VulkanRenderTarget::VulkanRenderTarget(RenderTarget::CreateInfo& createInfo) : format(createInfo.format), width(createInfo.width), height(createInfo.height), isSampled(createInfo.isSampled), isOwnedBySwapchain(false), isWrittenByCompute(isWrittenByCompute), hasMipChain(hasMipChain){
	if (createInfo.debugName != nullptr) {
		debugName = createInfo.debugName;
	}

	Create();
}

void VulkanRenderTarget::Create() {
	uint8_t channels;
	VkFormat renderFormat = TranslateColorFormatToVulkan(format, channels);

	uint32_t mipLevels = 1;

	VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	if (isSampled) {
		imageUsageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}

	if (isWrittenByCompute) {
		imageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	CreateImage(
		width,
		height,
		mipLevels,
		renderFormat,
		VK_IMAGE_TILING_OPTIMAL,
		imageUsageFlags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image,
		imageMemory
	);
	imageView = CreateImageView(image, renderFormat, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

	if (!debugName.empty()) {
		std::string imageViewDebugName = debugName + " View";
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_IMAGE, image, debugName.c_str());
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_IMAGE_VIEW, imageView, imageViewDebugName.c_str());
	}

	if (isSampled) {
		TransitionImageLayout(
			image,
			renderFormat,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			mipLevels,
			1
		);

		CreateTextureSampler();

		if (!debugName.empty()) {
			std::string imageSamplerDebugName = debugName + " Sampler";
			VulkanCore::Get().NameObject(VK_OBJECT_TYPE_SAMPLER, sampler, imageSamplerDebugName.c_str());
		}
	}
}

VulkanRenderTarget::~VulkanRenderTarget() {
	Cleanup();
}

void VulkanRenderTarget::Cleanup() {
	VkDevice device = VulkanCore::Get().GetDevice();

	if (sampler != nullptr) {
		vkDestroySampler(device, sampler, nullptr);
		sampler = nullptr;
	}

	if (imageView != nullptr) {
		vkDestroyImageView(device, imageView, nullptr);
		imageView = nullptr;
	}

	if (!isOwnedBySwapchain) {
		if (image != nullptr) {
			vkDestroyImage(device, image, nullptr);
			image = nullptr;
		}

		if (imageMemory != nullptr) {
			vkFreeMemory(device, imageMemory, nullptr);
			imageMemory = nullptr;
		}
	}
}

void VulkanRenderTarget::CreateTextureSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
	samplerInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0;
	samplerInfo.maxLod = 1;
	samplerInfo.mipLodBias = 0;

	if (vkCreateSampler(VulkanCore::Get().GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void VulkanRenderTarget::UpdateSwapChainImage(VkImage swapchainImage) {
	image = swapchainImage;

	VkDevice device = VulkanCore::Get().GetDevice();

	if (imageView != nullptr) {
		vkDestroyImageView(device, imageView, nullptr);
		imageView = nullptr;
	}
}

VkImage VulkanRenderTarget::GetImage() {
	return image;
}

VkImageView VulkanRenderTarget::GetImageView() {
	return imageView;
}

VkSampler VulkanRenderTarget::GetSampler() {
	return sampler;
}

void VulkanRenderTarget::Resize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;

	Cleanup();
	Create();
}

void VulkanRenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char * data) {
	std::cout << "VulkanRenderTarget::RenderScreen is not used.\n";
	assert(false);
}
