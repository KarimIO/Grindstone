#pragma once

#include "../GraphicsCommon/Texture.hpp"
#include <vulkan/vulkan.h>

class VulkanTextureBindingLayout : public TextureBindingLayout {
	VkDescriptorSetLayout descriptorSetLayout;
	VkDevice *device;
public:
	VulkanTextureBindingLayout(VkDevice *device, TextureBindingLayoutCreateInfo ci);
	VkDescriptorSetLayout *getLayout();
	~VulkanTextureBindingLayout();
};

class VulkanTextureBinding : public TextureBinding {
	VkDescriptorSet descriptorSet;
	VkDevice *device;
public:
	VulkanTextureBinding(VkDevice *device, VkDescriptorPool *descriptorPool, TextureBindingCreateInfo);
	VkDescriptorSet *getDescriptor();
};

class VulkanTexture : public Texture {
	VkDevice *device;
	VkPhysicalDevice *physicalDevice;

	VkImage image;
	VkImageView imageView;
	VkSampler sampler;
	VkDeviceMemory memory;

	uint32_t width, height;
public:
	VulkanTexture(VkDevice *dev, VkPhysicalDevice *phys, VkCommandPool *commandPool, VkDescriptorPool *descriptorPool, VkQueue *queue, TextureCreateInfo ci);
	VulkanTexture(VkDevice *dev, VkPhysicalDevice *phys, VkCommandPool *commandPool, VkDescriptorPool *descriptorPool, VkQueue *queue, CubemapCreateInfo ci);
	~VulkanTexture();

	VkImageView *GetImageView();
	VkSampler *GetSampler();
};