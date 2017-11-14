#pragma once

#include <vulkan/vulkan.h>

VkFormat TranslateFormat(uint32_t format);

uint32_t findMemoryType(VkPhysicalDevice *physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

void createBuffer(VkDevice *device, VkPhysicalDevice *physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

void createImage(VkDevice *device, VkPhysicalDevice *physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);