#include <iostream>
#include <cassert>

#include <EngineCore/Logger.hpp>

#include "VulkanRenderTarget.hpp"
#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"

namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::RenderTarget::RenderTarget(
	VkImage swapchainImage,
	VkFormat format,
	uint32_t swapchainIndex
) : image(swapchainImage),
	isOwnedBySwapchain(true)
{
	debugName = "Swapchain " + std::to_string(swapchainIndex);

	imageView = CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	std::string imageViewDebugName = debugName + " View";
	Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_IMAGE, image, debugName.c_str());
	Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_IMAGE_VIEW, imageView, imageViewDebugName.c_str());
}

Vulkan::RenderTarget::RenderTarget(VkImage image, VkImageView imageView, VkFormat colorFormat) : image(image), imageView(imageView), isOwnedBySwapchain(true) {}

Vulkan::RenderTarget::RenderTarget(const CreateInfo& createInfo) : format(createInfo.format), width(createInfo.width), height(createInfo.height), isSampled(createInfo.isSampled), isOwnedBySwapchain(false), isWrittenByCompute(createInfo.isWrittenByCompute), hasMipChain(createInfo.hasMipChain){
	if (createInfo.debugName != nullptr) {
		debugName = createInfo.debugName;
	}

	Create();
}

void Vulkan::RenderTarget::UpdateNativeImage(VkImage image, VkImageView imageView, VkFormat format) {
	isOwnedBySwapchain = true;
	this->image = image;
	this->imageView = imageView;
	this->format = TranslateColorFormatFromVulkan(format);
}

void Vulkan::RenderTarget::Create() {
	if (debugName.empty()) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Render Target!");
	}

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

	{
		std::string imageViewDebugName = debugName + " View";
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_IMAGE, image, debugName.c_str());
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_IMAGE_VIEW, imageView, imageViewDebugName.c_str());
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

		std::string imageSamplerDebugName = debugName + " Sampler";
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_SAMPLER, sampler, imageSamplerDebugName.c_str());
	}
}

Vulkan::RenderTarget::~RenderTarget() {
	Cleanup();
}

void Vulkan::RenderTarget::Cleanup() {
	VkDevice device = Vulkan::Core::Get().GetDevice();

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

void Vulkan::RenderTarget::CreateTextureSampler() {
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

	if (vkCreateSampler(Vulkan::Core::Get().GetDevice(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create texture sampler!");
	}
}

void Vulkan::RenderTarget::UpdateSwapChainImage(VkImage swapchainImage) {
	image = swapchainImage;

	VkDevice device = Vulkan::Core::Get().GetDevice();

	if (imageView != nullptr) {
		vkDestroyImageView(device, imageView, nullptr);
		imageView = nullptr;
	}
}

VkImage Vulkan::RenderTarget::GetImage() const {
	return image;
}

VkImageView Vulkan::RenderTarget::GetImageView() const {
	return imageView;
}

VkSampler Vulkan::RenderTarget::GetSampler() const {
	return sampler;
}

void Vulkan::RenderTarget::Resize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;

	Cleanup();
	Create();
}

void Vulkan::RenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char * data) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanRenderTarget::RenderScreen is not used.");
}
