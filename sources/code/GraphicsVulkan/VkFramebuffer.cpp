#include "VkFramebuffer.hpp"
#include "VkRenderPass.hpp"
#include "VkBufferCommon.hpp"
#include "VkFormats.hpp"
#include <array>
#include <iostream>

void VulkanFramebuffer::CreateDepthStencil(VkImage &image, VkImageView &imageView, VkDeviceMemory &imageMem, VkFormat imageFormat) {
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

VulkanFramebuffer::VulkanFramebuffer(VkDevice *dev, VkPhysicalDevice *_physicalDevice, VkImage image, VkFormat imageFormat, DefaultFramebufferCreateInfo createInfo) {
	device = dev;
	physicalDevice = _physicalDevice;
	width = createInfo.width;
	height = createInfo.height;
	defaultFramebuffer = true;

	if (createInfo != DepthFormat::None) {
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
	framebufferInfo.renderPass = *((VulkanRenderPass *)createInfo.renderPass)->GetRenderPass();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
	framebufferInfo.pAttachments = imageViews.data();
	framebufferInfo.width = createInfo.width;
	framebufferInfo.height = createInfo.height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(*device, &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}
}

VulkanFramebuffer::VulkanFramebuffer(VkDevice * device, VkPhysicalDevice * physicalDevice, FramebufferCreateInfo createInfo)
{
}

VulkanFramebuffer::~VulkanFramebuffer() {
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

VkFramebuffer *VulkanFramebuffer::GetFramebuffer() {
	return &framebuffer;
}

float VulkanFramebuffer::getExposure(int i) {
	std::cout << "VulkanFramebuffer::getExposure is not used in Vulkan\n";
	return 0.0f;
}

void VulkanFramebuffer::Clear(int mask) {
	std::cout << "VulkanFramebuffer::Clear is not used in Vulkan\n";
}

void VulkanFramebuffer::CopyFrom(Framebuffer *) {
	std::cout << "VulkanFramebuffer::CopyFrom is not used in Vulkan\n";
}

void VulkanFramebuffer::BindWrite(bool depth) {
	std::cout << "VulkanFramebuffer::BindWrite is not used in Vulkan\n";
}

void VulkanFramebuffer::BindTextures(int i) {
	std::cout << "VulkanFramebuffer::BindTextures is not used in Vulkan\n";
}

void VulkanFramebuffer::Bind(bool depth) {
	std::cout << "VulkanFramebuffer::Bind is not used in Vulkan\n";
}
