#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanDescriptorSet.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanDepthTarget.hpp"
#include "VulkanTexture.hpp"
#include "VulkanCore.hpp"

using namespace Grindstone::GraphicsAPI;

void AttachUniformBuffer(std::vector<VkWriteDescriptorSet>& writeVector, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	VulkanUniformBuffer* uniformBuffer = static_cast<VulkanUniformBuffer*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorBufferInfo* bufferInfo = new VkDescriptorBufferInfo();
	bufferInfo->buffer = uniformBuffer->GetBuffer();
	bufferInfo->offset = 0;
	bufferInfo->range = uniformBuffer->GetSize();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = binding.bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pBufferInfo = bufferInfo;
	writeVector.push_back(descriptorWrites);
}

void AttachTexture(std::vector<VkWriteDescriptorSet>& writeVector, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	VulkanTexture* texture = static_cast<VulkanTexture*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo->imageView = texture->GetImageView();
	imageInfo->sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = binding.bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = imageInfo;
	writeVector.push_back(descriptorWrites);
}

void AttachRenderTexture(std::vector<VkWriteDescriptorSet>& writeVector, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet, bool isStorage) {
	VulkanRenderTarget* texture = static_cast<VulkanRenderTarget*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageInfo->imageView = texture->GetImageView();
	imageInfo->sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = binding.bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = isStorage
		? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = imageInfo;
	writeVector.push_back(descriptorWrites);
}

void AttachDepthTexture(std::vector<VkWriteDescriptorSet>& writeVector, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	VulkanDepthTarget* texture = static_cast<VulkanDepthTarget*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	imageInfo->imageView = texture->GetImageView();
	imageInfo->sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = binding.bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = imageInfo;
	writeVector.push_back(descriptorWrites);
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

	VulkanCore::Get().NameObject(VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSet, createInfo.debugName);

	ChangeBindings(createInfo.bindings, createInfo.bindingCount);
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
		case BindingType::RenderTexture:
		case BindingType::RenderTextureStorageImage:
			AttachRenderTexture(descriptorWrites, binding, descriptorSet, binding.bindingType == BindingType::RenderTextureStorageImage);
			break;
		case BindingType::DepthTexture:
			AttachDepthTexture(descriptorWrites, binding, descriptorSet);
			break;
		}
	}

	vkUpdateDescriptorSets(
		VulkanCore::Get().GetDevice(),
		static_cast<uint32_t>(descriptorWrites.size()),
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
