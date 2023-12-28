#include <vector>
#include "VulkanCore.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanUtils.hpp"

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

	for (uint32_t i = 0; i < createInfo.bindingCount; ++i) {
		DescriptorSetLayout::Binding& sourceBinding = createInfo.bindings[i];

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
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	VulkanCore::Get().NameObject(VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, descriptorSetLayout, createInfo.debugName);
}

VkDescriptorSetLayout VulkanDescriptorSetLayout::GetInternalLayout() const {
	return descriptorSetLayout;
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
	vkDestroyDescriptorSetLayout(VulkanCore::Get().GetDevice(), descriptorSetLayout, nullptr);
}
