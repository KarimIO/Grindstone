#include <assert.h>
#include "VulkanTexture.hpp"
#include "VulkanUtils.hpp"
#include "VulkanFormat.hpp"
#include "VulkanCore.hpp"
#include <vulkan/vulkan.h>

using namespace Grindstone::GraphicsAPI;

VulkanTexture::VulkanTexture(Texture::CreateInfo& ci) {
	uint32_t mipLevels;
	CreateTextureImage(ci, mipLevels);
	imageView = CreateImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
	CreateTextureSampler(ci, mipLevels);
}

VulkanTexture::~VulkanTexture() {
	VkDevice device = VulkanCore::Get().GetDevice();
	vkDestroySampler(device, sampler, nullptr);
	vkDestroyImageView(device, imageView, nullptr);

	vkDestroyImage(device, image, nullptr);
	vkFreeMemory(device, imageMemory, nullptr);
}

void VulkanTexture::CreateTextureImage(Texture::CreateInfo& createInfo, uint32_t &mipLevels) {
	VkDevice device = VulkanCore::Get().GetDevice();

	uint8_t channels = 4;
	format = TranslateColorFormatToVulkan(createInfo.format, channels);

	mipLevels = createInfo.mipmaps;

	uint32_t imageSize = createInfo.size;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, createInfo.data, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	CreateImage(createInfo.width, createInfo.height, mipLevels, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

	TransitionImageLayout(image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	CopyBufferToImage(stagingBuffer, image, static_cast<uint32_t>(createInfo.width), static_cast<uint32_t>(createInfo.height));
	TransitionImageLayout(image, format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	//if (createInfo.options.generate_mipmaps)
	//	generateMipmaps(image_, VK_FORMAT_R8G8B8A8_UNORM, createInfo.width, createInfo.height, mipLevels);
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
