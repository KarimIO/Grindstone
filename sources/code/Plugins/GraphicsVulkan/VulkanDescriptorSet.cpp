#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanDescriptorSet.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanCore.hpp"

using namespace Grindstone::GraphicsAPI;

void AttachUniformBuffer(std::vector<VkWriteDescriptorSet>& writeVector, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	VulkanUniformBuffer* uniformBuffer = static_cast<VulkanUniformBuffer*>(binding.itemPtr);

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = uniformBuffer->GetBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = uniformBuffer->GetSize();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = binding.bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pBufferInfo = &bufferInfo;
}

void AttachTexture(std::vector<VkWriteDescriptorSet>& writeVector, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	VulkanTexture* texture = static_cast<VulkanTexture*>(binding.itemPtr);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture->GetImageView();
	imageInfo.sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = 0;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = &imageInfo;
}

VulkanDescriptorSet::VulkanDescriptorSet(DescriptorSet::CreateInfo& createInfo) {
	VulkanDescriptorSetLayout* layout = static_cast<VulkanDescriptorSetLayout*>(createInfo.layout);
	VkDescriptorSetLayout internalLayout = layout->GetInternalLayout();

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VulkanCore::Get().descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &internalLayout;

	if (vkAllocateDescriptorSets(VulkanCore::Get().GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}

void VulkanDescriptorSet::ChangeBindings(Binding* bindings, uint32_t bindingCount) {
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	for (uint32_t i = 0; i < bindingCount; ++i) {
		Binding& binding = bindings[i];

		if (binding.itemPtr == nullptr) {
			continue;
		}

		switch (binding.bindingType) {
		case BindingType::UniformBuffer:
			AttachUniformBuffer(descriptorWrites, binding, descriptorSet);
			break;
		case BindingType::Texture:
			AttachTexture(descriptorWrites, binding, descriptorSet);
			break;
		}
	}

	// We use this instead of bindingCount in case any bindings are invalid
	uint32_t descriptorWriteCount = static_cast<uint32_t>(descriptorWrites.size());

	vkUpdateDescriptorSets(
		VulkanCore::Get().GetDevice(),
		descriptorWriteCount,
		descriptorWrites.data(),
		0,
		nullptr
	);
}

VulkanDescriptorSet::~VulkanDescriptorSet() {
}

VkDescriptorSet VulkanDescriptorSet::GetDescriptorSet() {
	return descriptorSet;
}
