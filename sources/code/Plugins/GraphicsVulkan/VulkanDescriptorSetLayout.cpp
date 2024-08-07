#include <vector>

#include <EngineCore/Logger.hpp>

#include "VulkanCore.hpp"
#include "VulkanUtils.hpp"
#include "VulkanDescriptorSetLayout.hpp"

using namespace Grindstone::GraphicsAPI;

static VkDescriptorType GetDescriptorType(BindingType bindingType) {
	switch (bindingType) {
	case BindingType::UniformBuffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case BindingType::Texture:
	case BindingType::RenderTexture:
	case BindingType::DepthTexture:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case BindingType::RenderTextureStorageImage:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	}

	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(DescriptorSetLayout::CreateInfo& createInfo) {
	std::vector<VkDescriptorSetLayoutBinding> bindingLayouts;
	bindingLayouts.reserve(createInfo.bindingCount);

	bindings = new Binding[createInfo.bindingCount];
	bindingCount = createInfo.bindingCount;

	for (uint32_t i = 0; i < createInfo.bindingCount; ++i) {
		DescriptorSetLayout::Binding& sourceBinding = createInfo.bindings[i];
		bindings[i] = sourceBinding;

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

	if (vkCreateDescriptorSetLayout(VulkanCore::Get().GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create descriptor set layout!");
	}

	if (createInfo.debugName != nullptr) {
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayout, createInfo.debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Descriptor Set Layout!");
	}
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::GetInternalLayout() const {
	return descriptorSetLayout;
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
	vkDestroyDescriptorSetLayout(VulkanCore::Get().GetDevice(), descriptorSetLayout, nullptr);
}

const DescriptorSetLayout::Binding& VulkanDescriptorSetLayout::GetBinding(size_t bindingIndex) const {
	if (bindings == nullptr || bindingIndex >= bindingCount) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Invalid bindingIndex in GetBinding!");
	}

	return bindings[bindingIndex];
}
