#include "VulkanDepthTarget.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUtils.hpp"
#include <assert.h>

using namespace Grindstone::GraphicsAPI;

VulkanDepthTarget::VulkanDepthTarget(DepthTarget::CreateInfo& createInfo) : format(createInfo.format), width(createInfo.width), height(createInfo.height), isShadowMap(createInfo.isShadowMap), isCubemap(createInfo.isCubemap), isSampled(createInfo.isSampled) {
	if (createInfo.debugName != nullptr) {
		debugName = createInfo.debugName;
	}

	Create();
}

void VulkanDepthTarget::Create() {
	bool hasStencil = false;
	VkFormat depthFormat = TranslateDepthFormatToVulkan(format, hasStencil);

	VkImageAspectFlags aspectFlags = hasStencil
		? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT
		: VK_IMAGE_ASPECT_DEPTH_BIT;

	VkImageUsageFlags imageUsageFlags = isSampled
		? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		: VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	CreateImage(width, height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, imageUsageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
	imageView = CreateImageView(image, depthFormat, aspectFlags, 1);

	if (!debugName.empty()) {
		std::string imageViewDebugName = debugName + " View";
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_IMAGE, image, debugName.c_str());
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_IMAGE_VIEW, imageView, imageViewDebugName.c_str());
	}

	if (isSampled) {
		VkImageLayout newLayout = hasStencil
			? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
			: VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;

		TransitionImageLayout(
			image,
			depthFormat,
			VK_IMAGE_ASPECT_DEPTH_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			newLayout,
			1,
			1
		);

		CreateTextureSampler();

		if (!debugName.empty()) {
			std::string imageSamplerDebugName = debugName + " Sampler";
			VulkanCore::Get().NameObject(VK_OBJECT_TYPE_SAMPLER, sampler, debugName.c_str());
		}
	}
}
		
VulkanDepthTarget::~VulkanDepthTarget() {
	Cleanup();
}

void VulkanDepthTarget::Cleanup() {
	VkDevice device = VulkanCore::Get().GetDevice();
	if (sampler != nullptr) {
		vkDestroySampler(device, sampler, nullptr);
		sampler = nullptr;
	}

	if (imageView != nullptr) {
		vkDestroyImageView(device, imageView, nullptr);
		imageView = nullptr;
	}

	if (image != nullptr) {
		vkDestroyImage(device, image, nullptr);
		image = nullptr;
	}

	if (imageMemory != nullptr) {
		vkFreeMemory(device, imageMemory, nullptr);
		imageMemory = nullptr;
	}
}

void VulkanDepthTarget::CreateTextureSampler() {
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

VkImageView VulkanDepthTarget::GetImageView() const {
	return imageView;
}

VkSampler VulkanDepthTarget::GetSampler() const {
	return sampler;
}

void VulkanDepthTarget::Resize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
	Cleanup();
	Create();
}

void VulkanDepthTarget::BindFace(int k) {
	std::cout << "VulkanDepthTarget::BindFace is not used.\n";
	assert(false);
}
