#include <vector>

#include <EngineCore/Logger.hpp>

#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"
#include "VulkanDescriptorSetLayout.hpp"

using namespace Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

static VkDescriptorType GetDescriptorType(BindingType bindingType) {
	switch (bindingType) {
	case BindingType::Sampler:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case BindingType::CombinedImageSampler:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case BindingType::SampledImage:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case BindingType::StorageImage:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case BindingType::UniformTexelBuffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	case BindingType::StorageTexelBuffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case BindingType::UniformBuffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case BindingType::StorageBuffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case BindingType::UniformBufferDynamic:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case BindingType::StorageBufferDynamic:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	}

	return VK_DESCRIPTOR_TYPE_MAX_ENUM;
}

Vulkan::DescriptorSetLayout::DescriptorSetLayout(const CreateInfo& createInfo) {
	uint32_t count = 0;
	for (uint32_t i = 0; i < createInfo.bindingCount; ++i) {
		DescriptorSetLayout::Binding& sourceBinding = createInfo.bindings[i];
		uint32_t testCount = sourceBinding.bindingId + 1;
		if (testCount > count) {
			count = testCount;
		}
	}

	std::vector<VkDescriptorSetLayoutBinding> bindingLayouts;
	bindingLayouts.reserve(createInfo.bindingCount);

	bindings.resize(count);
	bindingCount = count;

	for (uint32_t i = 0; i < count; ++i) {
		bindings[i].bindingId = i;
		bindings[i].count = 0;
		bindings[i].stages = Grindstone::GraphicsAPI::ShaderStageBit::None;
		bindings[i].type = Grindstone::GraphicsAPI::BindingType::None;
	}

	for (uint32_t i = 0; i < createInfo.bindingCount; ++i) {
		DescriptorSetLayout::Binding& sourceBinding = createInfo.bindings[i];
		bindings[sourceBinding.bindingId] = sourceBinding;

		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = sourceBinding.bindingId;
		binding.descriptorCount = sourceBinding.count;
		binding.descriptorType = GetDescriptorType(sourceBinding.type);
		binding.stageFlags = TranslateShaderStageBits(sourceBinding.stages);

		bindingLayouts.emplace_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindingLayouts.size());
	layoutInfo.pBindings = bindingLayouts.data();

	if (vkCreateDescriptorSetLayout(Vulkan::Core::Get().GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create descriptor set layout!");
	}

	if (createInfo.debugName != nullptr) {
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayout, createInfo.debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Descriptor Set Layout!");
	}
}

VkDescriptorSetLayout Vulkan::DescriptorSetLayout::GetInternalLayout() const {
	return descriptorSetLayout;
}

Vulkan::DescriptorSetLayout::~DescriptorSetLayout() {
	vkDestroyDescriptorSetLayout(Vulkan::Core::Get().GetDevice(), descriptorSetLayout, nullptr);
}

const DescriptorSetLayout::Binding& Vulkan::DescriptorSetLayout::GetBinding(size_t bindingIndex) const {
	if (bindings.empty() || bindingIndex >= bindingCount) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Invalid bindingIndex in GetBinding!");
	}

	return bindings[bindingIndex];
}
