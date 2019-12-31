#include "VkUniformBuffer.hpp"
#include <iostream>
#include "VkBufferCommon.hpp"
#include <vector>
#include <cstring>

vkUniformBuffer::vkUniformBuffer(VkDevice * dev, VkPhysicalDevice * phys, VkDescriptorPool *descPool, UniformBufferCreateInfo ci) {
	device = dev;
	physicalDevice = phys;
	descriptorPool = descPool;
	size = ci.size;

	VkDeviceSize bufferSize = size;
	createBuffer(device, physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, memory);

	vkUniformBufferBinding *binding = (vkUniformBufferBinding *)ci.binding;
	VkDescriptorSetLayout layouts[] = { *binding->getLayout() };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = *descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(*device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = size;

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(*device, 1, &descriptorWrite, 0, nullptr);
}

void vkUniformBuffer::UpdateUniformBuffer(void *content) {
	void* data;
	vkMapMemory(*device, memory, 0, size, 0, &data);
	std::memcpy(data, content, size);
	vkUnmapMemory(*device, memory);
}

VkDescriptorSet * vkUniformBuffer::GetDescriptorSet() {
	return &descriptorSet;
}

vkUniformBuffer::~vkUniformBuffer() {
	vkDestroyBuffer(*device, buffer, nullptr);
	vkFreeMemory(*device, memory, nullptr);
}

vkUniformBufferBinding::vkUniformBufferBinding(VkDevice *device, UniformBufferBindingCreateInfo ci) {
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = ci.binding;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = ci.stages;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

VkDescriptorSetLayout * vkUniformBufferBinding::getLayout() {
	return &descriptorSetLayout;
}

vkUniformBufferBinding::~vkUniformBufferBinding() {
	vkDestroyDescriptorSetLayout(*device, descriptorSetLayout, nullptr);
}
