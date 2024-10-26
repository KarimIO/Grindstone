#include <EngineCore/Logger.hpp>

#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanDepthStencilTarget.hpp"
#include "VulkanTexture.hpp"
#include "VulkanCore.hpp"

#include "VulkanDescriptorSet.hpp"

namespace Base = Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

static void AttachUniformBuffer(std::vector<VkWriteDescriptorSet>& writeVector, uint32_t bindingIndex, Base::DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	Vulkan::UniformBuffer* uniformBuffer = static_cast<Vulkan::UniformBuffer*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorBufferInfo* bufferInfo = new VkDescriptorBufferInfo();
	bufferInfo->buffer = uniformBuffer->GetBuffer();
	bufferInfo->offset = 0;
	bufferInfo->range = uniformBuffer->GetSize();

	VkWriteDescriptorSet descriptorWrites{};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pBufferInfo = bufferInfo;
	writeVector.push_back(descriptorWrites);
}

static void AttachTexture(std::vector<VkWriteDescriptorSet>& writeVector, uint32_t bindingIndex, Base::DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	Vulkan::Texture* texture = static_cast<Vulkan::Texture*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo->imageView = texture->GetImageView();
	imageInfo->sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites{};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = imageInfo;
	writeVector.push_back(descriptorWrites);
}

static void AttachRenderTexture(std::vector<VkWriteDescriptorSet>& writeVector, uint32_t bindingIndex, Base::DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet, bool isStorage) {
	Vulkan::RenderTarget* texture = static_cast<Vulkan::RenderTarget*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = isStorage
		? VK_IMAGE_LAYOUT_GENERAL
		: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo->imageView = texture->GetImageView();
	imageInfo->sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites{};
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

static void AttachDepthTexture(std::vector<VkWriteDescriptorSet>& writeVector, uint32_t bindingIndex, Base::DescriptorSet::Binding& binding, VkDescriptorSet descriptorSet) {
	Vulkan::DepthStencilTarget* texture = static_cast<Vulkan::DepthStencilTarget*>(binding.itemPtr);

	// TODO: Handle this lifetime
	VkDescriptorImageInfo* imageInfo = new VkDescriptorImageInfo();
	imageInfo->imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	imageInfo->imageView = texture->GetImageView();
	imageInfo->sampler = texture->GetSampler();

	VkWriteDescriptorSet descriptorWrites{};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = imageInfo;
	writeVector.push_back(descriptorWrites);
}

Vulkan::DescriptorSet::DescriptorSet(const CreateInfo& createInfo) {
	layout = static_cast<Vulkan::DescriptorSetLayout*>(createInfo.layout);
	VkDescriptorSetLayout internalLayout = layout->GetInternalLayout();

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Vulkan::Core::Get().descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &internalLayout;

	if (vkAllocateDescriptorSets(Vulkan::Core::Get().GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to allocate descriptor sets!");
	}

	if (createInfo.debugName != nullptr) {
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_DESCRIPTOR_SET, descriptorSet, createInfo.debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Descriptor Set!");
	}

	ChangeBindings(createInfo.bindings, createInfo.bindingCount);
}

void Vulkan::DescriptorSet::ChangeBindings(Binding* sourceBindings, uint32_t bindingCount, uint32_t bindOffset) {
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	for (uint32_t i = 0; i < bindingCount; ++i) {
		const Vulkan::DescriptorSetLayout::Binding& layoutBinding = layout->GetBinding(static_cast<size_t>(bindOffset) + i);
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
		Vulkan::Core::Get().GetDevice(),
		static_cast<uint32_t>(descriptorWrites.size()),
		descriptorWrites.data(),
		0,
		nullptr
	);
}

Vulkan::DescriptorSet::~DescriptorSet() {
}

VkDescriptorSet Vulkan::DescriptorSet::GetDescriptorSet() const {
	return descriptorSet;
}
