#include "VkFramebuffer.h"
#include "VkRenderPass.h"
#include "VkBufferCommon.h"
#include "VkFormats.h"
#include <array>
#include <iostream>

void vkFramebuffer::CreateDepthStencil(VkImage &image, VkImageView &imageView, VkDeviceMemory &imageMem, VkFormat imageFormat) {
	createImage(device, physicalDevice, width, height, imageFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMem);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = imageFormat;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(*device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}

vkFramebuffer::vkFramebuffer(VkDevice *dev, VkPhysicalDevice *_physicalDevice, VkImage image, VkFormat imageFormat, DefaultFramebufferCreateInfo createInfo) {
	device = dev;
	physicalDevice = _physicalDevice;
	width = createInfo.width;
	height = createInfo.height;
	defaultFramebuffer = true;

	if (createInfo.depthFormat != FORMAT_DEPTH_NONE) {
		images.resize(2);
		imageViews.resize(2);
		imageMemory.resize(1);

		CreateDepthStencil(images[1], imageViews[1], imageMemory[0], TranslateDepthFormat(createInfo.depthFormat));
	}
	else {
		images.resize(1);
		imageViews.resize(1);
	}

	images[0] = image;

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = images[0];
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = imageFormat;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(*device, &imageViewCreateInfo, nullptr, &imageViews[0]) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image views!");
	}

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = *((vkRenderPass *)createInfo.renderPass)->GetRenderPass();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
	framebufferInfo.pAttachments = imageViews.data();
	framebufferInfo.width = createInfo.width;
	framebufferInfo.height = createInfo.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}

vkFramebuffer::vkFramebuffer(VkDevice * device, VkPhysicalDevice * physicalDevice, FramebufferCreateInfo createInfo)
{
}

void vkFramebuffer::Blit(int i, int x, int y, int w, int h) {
	std::cout << "vkFramebuffer::Blit is not used in Vulkan\n";
}

void vkFramebuffer::BindWrite() {
	std::cout << "vkFramebuffer::BindWrite is not used in Vulkan\n";
}

void vkFramebuffer::BindRead() {
	std::cout << "vkFramebuffer::BindRead is not used in Vulkan\n";
}

void vkFramebuffer::Clear() {
	std::cout << "vkFramebuffer::Clear is not used in Vulkan\n";
}

void vkFramebuffer::BindTextures() {
	std::cout << "vkFramebuffer::BindTextures is not used in Vulkan\n";
}

void vkFramebuffer::Unbind() {
	std::cout << "vkFramebuffer::Unbind is not used in Vulkan\n";
}

vkFramebuffer::~vkFramebuffer() {
	vkDestroyFramebuffer(*device, framebuffer, nullptr);
	
	size_t i = 0;
	for (; i < imageViews.size(); i++) {
		vkDestroyImageView(*device, imageViews[i], nullptr);
	}

	for (i = defaultFramebuffer ? 1 : 0; i < images.size(); i++) {
		vkDestroyImage(*device, images[i], nullptr);
	}

	for (i = 0; i < imageMemory.size(); i++) {
		vkFreeMemory(*device, imageMemory[i], nullptr);
	}
}

VkFramebuffer *vkFramebuffer::GetFramebuffer() {
	return &framebuffer;
}
