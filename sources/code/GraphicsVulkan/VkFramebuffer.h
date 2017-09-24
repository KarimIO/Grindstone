#pragma once

#include <vulkan/vulkan.h>
#include "../GraphicsCommon/Framebuffer.h"
#include "../GraphicsCommon/RenderPass.h"
#include <vector>

class vkFramebuffer : public Framebuffer {
	VkDevice *device;
	VkPhysicalDevice *physicalDevice;
	VkFramebuffer framebuffer;
	uint32_t width, height;

	bool defaultFramebuffer;

	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	std::vector<VkDeviceMemory> imageMemory;

	void CreateDepthStencil(VkImage & image, VkImageView & imageView, VkDeviceMemory &imageMem, VkFormat imageFormat);
public:
	vkFramebuffer(VkDevice *device, VkPhysicalDevice *physicalDevice, VkImage image, VkFormat imageFormat, DefaultFramebufferCreateInfo createInfo);
	vkFramebuffer(VkDevice *device, VkPhysicalDevice *physicalDevice, FramebufferCreateInfo createInfo);
	~vkFramebuffer();

	VkFramebuffer *GetFramebuffer();
	
	void Clear();
	void Blit(int i, int x, int y, int w, int h);
	void BindWrite();
	void BindRead();
	void BindTextures();
	void Unbind();
};