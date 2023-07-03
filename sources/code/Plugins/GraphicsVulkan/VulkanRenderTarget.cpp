#include "VulkanRenderTarget.hpp"
#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"
#include <iostream>
#include <cassert>

using namespace Grindstone::GraphicsAPI;

VulkanRenderTarget::VulkanRenderTarget(VkImage swapchainImage, VkFormat format) : image(swapchainImage) {
	imageView = CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

VulkanRenderTarget::VulkanRenderTarget(RenderTarget::CreateInfo& createInfo) {
	uint8_t channels;
	VkFormat renderFormat = TranslateColorFormatToVulkan(createInfo.format, channels);

	uint32_t mipLevels = 1;

	VkImageUsageFlags imageUsageFlags = createInfo.isSampled
		? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		: VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	CreateImage(
		createInfo.width,
		createInfo.height,
		mipLevels,
		renderFormat,
		VK_IMAGE_TILING_OPTIMAL,
		imageUsageFlags,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image,
		imageMemory
	);
	imageView = CreateImageView(image, renderFormat, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

	if (createInfo.isSampled) {
		TransitionImageLayout(
			image,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			mipLevels
		);

		CreateTextureSampler();
	}
}

VulkanRenderTarget::~VulkanRenderTarget() {
	VkDevice device = VulkanCore::Get().GetDevice();
	vkDestroyImageView(device, imageView, nullptr);
	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
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

VkImageView VulkanRenderTarget::GetImageView() {
	return imageView;
}

VkSampler VulkanRenderTarget::GetSampler() {
	return sampler;
}

void VulkanRenderTarget::Resize(uint32_t width, uint32_t height) {
}

void VulkanRenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char * data) {
	std::cout << "VulkanRenderTarget::RenderScreen is not used.\n";
	assert(false);
}
