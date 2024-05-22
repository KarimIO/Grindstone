#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanDescriptorSet.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanDepthTarget.hpp"
#include "VulkanTexture.hpp"
#include "VulkanCore.hpp"

using namespace Grindstone::GraphicsAPI;

static void AttachUniformBuffer(std::vector<VkWriteDescriptorSet>& writeVector, uint32_t bindingIndex, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	VulkanUniformBuffer* uniformBuffer = static_cast<VulkanUniformBuffer*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorBufferInfo* bufferInfo = new VkDescriptorBufferInfo();
	bufferInfo->buffer = uniformBuffer->GetBuffer();
	bufferInfo->offset = 0;
	bufferInfo->range = uniformBuffer->GetSize();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pBufferInfo = bufferInfo;
	writeVector.push_back(descriptorWrites);
}

static void AttachTexture(std::vector<VkWriteDescriptorSet>& writeVector, uint32_t bindingIndex, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	VulkanTexture* texture = static_cast<VulkanTexture*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo->imageView = texture->GetImageView();
	imageInfo->sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = imageInfo;
	writeVector.push_back(descriptorWrites);
}

static void AttachRenderTexture(std::vector<VkWriteDescriptorSet>& writeVector, uint32_t bindingIndex, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet, bool isStorage) {
	VulkanRenderTarget* texture = static_cast<VulkanRenderTarget*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = isStorage
		? VK_IMAGE_LAYOUT_GENERAL
		: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo->imageView = texture->GetImageView();
	imageInfo->sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = isStorage
		? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		: VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = imageInfo;
	writeVector.push_back(descriptorWrites);
}

static void AttachDepthTexture(std::vector<VkWriteDescriptorSet>& writeVector, uint32_t bindingIndex, DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	VulkanDepthTarget* texture = static_cast<VulkanDepthTarget*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	imageInfo->imageView = texture->GetImageView();
	imageInfo->sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites = {};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = imageInfo;
	writeVector.push_back(descriptorWrites);
}

VulkanDescriptorSet::VulkanDescriptorSet(DescriptorSet::CreateInfo& createInfo) {
	layout = static_cast<VulkanDescriptorSetLayout*>(createInfo.layout);
	VkDescriptorSetLayout internalLayout = layout->GetInternalLayout();

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = VulkanCore::Get().descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &internalLayout;

	if (vkAllocateDescriptorSets(VulkanCore::Get().GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	if (createInfo.debugName != nullptr) {
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSet, createInfo.debugName);
	}
	else {
		throw std::runtime_error("Unnamed Descriptor Set!");
	}

	ChangeBindings(createInfo.bindings, createInfo.bindingCount);
}

void VulkanDescriptorSet::ChangeBindings(Binding* sourceBindings, uint32_t bindingCount, uint32_t bindOffset) {
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	for (uint32_t i = 0; i < bindingCount; ++i) {
		const VulkanDescriptorSetLayout::Binding& layoutBinding = layout->GetBinding(bindOffset + i);
		Binding& sourceBinding = sourceBindings[i];

		if (sourceBinding.itemPtr == nullptr) {
			continue;
		}

		switch (layoutBinding.type) {
		case BindingType::UniformBuffer:
			AttachUniformBuffer(descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet);
			break;
		case BindingType::Texture:
			AttachTexture(descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet);
			break;
		case BindingType::RenderTexture:
		case BindingType::RenderTextureStorageImage:
			AttachRenderTexture(descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet, layoutBinding.type == BindingType::RenderTextureStorageImage);
			break;
		case BindingType::DepthTexture:
			AttachDepthTexture(descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet);
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
