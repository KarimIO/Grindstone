#pragma once

#include <vulkan/vulkan.h>
#include "../GraphicsCommon/Framebuffer.hpp"
#include "../GraphicsCommon/RenderPass.hpp"
#include <vector>

class VulkanFramebuffer : public Framebuffer {
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
	VulkanFramebuffer(VkDevice *device, VkPhysicalDevice *physicalDevice, VkImage image, VkFormat imageFormat, DefaultFramebufferCreateInfo createInfo);
	VulkanFramebuffer(VkDevice *device, VkPhysicalDevice *physicalDevice, FramebufferCreateInfo createInfo);
	virtual ~VulkanFramebuffer() override;

	VkFramebuffer *GetFramebuffer();
	
	virtual float getExposure(int i) override;
	virtual void Clear(int mask) override;
	virtual void CopyFrom(Framebuffer *) override;
	virtual void BindWrite(bool depth) override;
	virtual void BindTextures(int i) override;
	virtual void Bind(bool depth) override;
	virtual void BindRead() override;
	virtual void Unbind() override;
};