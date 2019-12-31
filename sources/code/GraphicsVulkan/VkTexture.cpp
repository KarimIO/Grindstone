#include "VkTexture.hpp"
#include "VkBufferCommon.hpp"
#include <iostream>
#include "VkFormats.hpp"
#include <vector>
#include <cstring>

vkTexture::vkTexture(VkDevice *dev, VkPhysicalDevice *phys, VkCommandPool *commandPool, VkDescriptorPool *descriptorPool, VkQueue *queue, TextureCreateInfo ci) {
	width = ci.width;
	height = ci.height;
	device = dev;
	physicalDevice = phys;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	uint32_t channels;

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
	switch (ci.format) {
	case ColorFormat::R8:
		format = VK_FORMAT_R8_UNORM;
		channels = 1;
		break;
	case ColorFormat::R8G8:
		format = VK_FORMAT_R8G8_UNORM;
		channels = 2;
		break;
	case ColorFormat::R8G8B8:
		format = VK_FORMAT_B8G8R8_UINT;
		channels = 3;
		break;
	case ColorFormat::R8G8B8A8:
		format = VK_FORMAT_R8G8B8A8_UNORM;
		channels = 4;
		break;
	default:
		std::cerr << "Failed to create Texture\n";
	}

	uint32_t size = ci.width * ci.height * channels;
	createBuffer(dev, phys, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(*device, stagingBufferMemory, 0, size, 0, &data);
	std::memcpy(data, ci.data, static_cast<size_t>(size));
	vkUnmapMemory(*device, stagingBufferMemory);

	createImage(dev, phys, ci.width, ci.height, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, memory);

	VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer(device, commandPool);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommandBuffer(device, commandPool, queue, commandBuffer);

	vkDestroyBuffer(*device, stagingBuffer, nullptr);
	vkFreeMemory(*device, stagingBufferMemory, nullptr);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(*device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(*device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

vkTexture::vkTexture(VkDevice * dev, VkPhysicalDevice * phys, VkCommandPool * commandPool, VkDescriptorPool * descriptorPool, VkQueue * queue, CubemapCreateInfo ci) {
}

vkTexture::~vkTexture() {
	vkDestroySampler(*device, sampler, nullptr);
	vkDestroyImageView(*device, imageView, nullptr);
	vkDestroyImage(*device, image, nullptr);
	vkFreeMemory(*device, memory, nullptr);
}

VkImageView * vkTexture::GetImageView() {
	return &imageView;
}

VkSampler * vkTexture::GetSampler() {
	return &sampler;
}

vkTextureBinding::vkTextureBinding(VkDevice *_device, VkDescriptorPool *descriptorPool, TextureBindingCreateInfo ci) {
	device = _device;
	
	vkTextureBindingLayout *binding = (vkTextureBindingLayout *)ci.textures;
	VkDescriptorSetLayout layouts[] = { *binding->getLayout() };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = *descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(*device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	vkTexture **texture = (vkTexture **)ci.textures;
	std::vector<VkDescriptorImageInfo> imageInfos;
	imageInfos.resize(ci.textureCount);
	for (uint32_t i = 0; i < ci.textureCount; i++) {
		imageInfos[i] = {};
		imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[i].imageView = *texture[i]->GetImageView();
		imageInfos[i].sampler = *texture[i]->GetSampler();
	}

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = imageInfos.size();
	descriptorWrite.pImageInfo = imageInfos.data();

	vkUpdateDescriptorSets(*device, 1, &descriptorWrite, 0, nullptr);
}

VkDescriptorSet * vkTextureBinding::getDescriptor() {
	return &descriptorSet;
}


vkTextureBindingLayout::vkTextureBindingLayout(VkDevice * device, TextureBindingLayoutCreateInfo ci) {
	VkDescriptorSetLayoutBinding layoutBinding = {};
	//layoutBinding.binding = ci.binding;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBinding.descriptorCount = 1;
	layoutBinding.stageFlags = ci.stages;
	layoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &layoutBinding;

	if (vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

VkDescriptorSetLayout *vkTextureBindingLayout::getLayout() {
	return &descriptorSetLayout;
}

vkTextureBindingLayout::~vkTextureBindingLayout() {
	vkDestroyDescriptorSetLayout(*device, descriptorSetLayout, nullptr);
}
