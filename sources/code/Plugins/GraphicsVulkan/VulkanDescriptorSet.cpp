#include <EngineCore/Logger.hpp>

#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanImage.hpp"
#include "VulkanSampler.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanCore.hpp"

#include "VulkanDescriptorSet.hpp"

namespace Base = Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

static void AttachUniformBuffer(
	std::vector<VkDescriptorBufferInfo>& descriptorBuffersInfos,
	std::vector<VkWriteDescriptorSet>& writeVector,
	uint32_t bindingIndex,
	const Base::DescriptorSet::Binding& binding,
	VkDescriptorSet descriptorSet,
	bool isStorageBuffer
) {
	Vulkan::Buffer* uniformBuffer = static_cast<Vulkan::Buffer*>(binding.itemPtr);

	VkDescriptorBufferInfo& bufferInfo = descriptorBuffersInfos.emplace_back();
	bufferInfo.buffer = uniformBuffer->GetBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = uniformBuffer->GetSize();

	VkWriteDescriptorSet descriptorWrites{};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = isStorageBuffer
		? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
		: VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pBufferInfo = &bufferInfo;
	writeVector.push_back(descriptorWrites);
}

static void AttachImage(
	std::vector<VkDescriptorImageInfo>& descriptorImageInfos,
	std::vector<VkWriteDescriptorSet>& writeVector,
	uint32_t bindingIndex,
	const Base::DescriptorSet::Binding& binding,
	VkDescriptorSet descriptorSet,
	bool isStorageImage
) {
	Vulkan::Image* image = static_cast<Vulkan::Image*>(binding.itemPtr);

	VkDescriptorImageInfo& imageInfo = descriptorImageInfos.emplace_back();
	imageInfo.imageLayout = isStorageImage
		? VK_IMAGE_LAYOUT_GENERAL
		: VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = image->GetImageView();
	imageInfo.sampler = nullptr;

	VkWriteDescriptorSet descriptorWrites{};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = isStorageImage
		? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
		: VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = &imageInfo;
	writeVector.push_back(descriptorWrites);
}

static void AttachSampler(
	std::vector<VkDescriptorImageInfo>& descriptorImageInfos,
	std::vector<VkWriteDescriptorSet>& writeVector,
	uint32_t bindingIndex,
	const Base::DescriptorSet::Binding& binding,
	VkDescriptorSet descriptorSet
) {
	Vulkan::Sampler* sampler = static_cast<Vulkan::Sampler*>(binding.itemPtr);

	VkDescriptorImageInfo& imageInfo = descriptorImageInfos.emplace_back();
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.imageView = nullptr;
	imageInfo.sampler = sampler->GetSampler();

	VkWriteDescriptorSet descriptorWrites{};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = &imageInfo;
	writeVector.push_back(descriptorWrites);
}

static void AttachCombinedImageSampler(
	std::vector<VkDescriptorImageInfo>& descriptorImageInfos,
	std::vector<VkWriteDescriptorSet>& writeVector,
	uint32_t bindingIndex,
	const Base::DescriptorSet::Binding& binding,
	VkDescriptorSet descriptorSet
) {
	std::pair<void*, void*>* samplerPair = static_cast<std::pair<void*, void*>*>(binding.itemPtr);
	Vulkan::Image* image = static_cast<Vulkan::Image*>(samplerPair->first);
	Vulkan::Sampler* sampler = static_cast<Vulkan::Sampler*>(samplerPair->second);

	VkDescriptorImageInfo& imageInfo = descriptorImageInfos.emplace_back();
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = image->GetImageView();
	imageInfo.sampler = sampler->GetSampler();

	VkWriteDescriptorSet descriptorWrites{};
	descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.dstSet = descriptorSet;
	descriptorWrites.dstBinding = bindingIndex;
	descriptorWrites.dstArrayElement = 0;
	descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites.descriptorCount = binding.count;
	descriptorWrites.pImageInfo = &imageInfo;
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

void Vulkan::DescriptorSet::ChangeBindings(const Binding* sourceBindings, uint32_t bindingCount, uint32_t bindOffset) {
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	// Allocate vectors bigger than the size we need so we can have stable pointers to them.
	std::vector<VkDescriptorImageInfo> descriptorImageInfos;
	descriptorImageInfos.reserve(bindingCount);
	std::vector<VkDescriptorBufferInfo> descriptorBufferInfos;
	descriptorBufferInfos.reserve(bindingCount);

	for (uint32_t i = 0; i < bindingCount; ++i) {
		const Vulkan::DescriptorSetLayout::Binding& layoutBinding = layout->GetBinding(static_cast<size_t>(bindOffset) + i);
		const Binding& sourceBinding = sourceBindings[i];

		if (sourceBinding.itemPtr == nullptr) {
			continue;
		}

		GS_ASSERT(sourceBinding.bindingType == layoutBinding.type);

		switch (layoutBinding.type) {
		case BindingType::Sampler:
			AttachSampler(descriptorImageInfos, descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet);
			break;
		case BindingType::SampledImage:
			AttachImage(descriptorImageInfos, descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet, false);
			break;
		case BindingType::StorageImage:
			AttachImage(descriptorImageInfos, descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet, true);
			break;
		case BindingType::StorageBuffer:
			AttachUniformBuffer(descriptorBufferInfos, descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet, true);
			break;
		case BindingType::UniformBuffer:
			AttachUniformBuffer(descriptorBufferInfos, descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet, false);
			break;
		case BindingType::CombinedImageSampler:
			AttachCombinedImageSampler(descriptorImageInfos, descriptorWrites, layoutBinding.bindingId, sourceBinding, descriptorSet);
			break;
		default:
			GS_BREAK_WITH_MESSAGE("Invalid BindingType");
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
