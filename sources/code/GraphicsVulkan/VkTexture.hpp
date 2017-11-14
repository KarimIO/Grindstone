#pragma once

#include "../GraphicsCommon/Texture.h"
#include <vulkan/vulkan.h>

class vkTextureBindingLayout : public TextureBindingLayout {
	VkDescriptorSetLayout descriptorSetLayout;
	VkDevice *device;
public:
	vkTextureBindingLayout(VkDevice *device, TextureBindingLayoutCreateInfo ci);
	VkDescriptorSetLayout *getLayout();
	~vkTextureBindingLayout();
};

class vkTextureBinding : public TextureBinding {
	VkDescriptorSet descriptorSet;
	VkDevice *device;
public:
	vkTextureBinding(VkDevice *device, VkDescriptorPool *descriptorPool, TextureBindingCreateInfo);
	VkDescriptorSet *getDescriptor();
};

class vkTexture : public Texture {
	VkDevice *device;
	VkPhysicalDevice *physicalDevice;

	VkImage image;
	VkImageView imageView;
	VkSampler sampler;
	VkDeviceMemory memory;

	uint32_t width, height;
public:
	vkTexture(VkDevice *dev, VkPhysicalDevice *phys, VkCommandPool *commandPool, VkDescriptorPool *descriptorPool, VkQueue *queue, TextureCreateInfo ci);
	vkTexture(VkDevice *dev, VkPhysicalDevice *phys, VkCommandPool *commandPool, VkDescriptorPool *descriptorPool, VkQueue *queue, CubemapCreateInfo ci);
	~vkTexture();

	VkImageView *GetImageView();
	VkSampler *GetSampler();
};