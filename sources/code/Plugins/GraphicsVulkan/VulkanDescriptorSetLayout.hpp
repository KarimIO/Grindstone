#pragma once

#include <Common/Graphics/DescriptorSetLayout.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI {
	class VulkanDescriptorSetLayout : public DescriptorSetLayout {
	public:
		VulkanDescriptorSetLayout(DescriptorSetLayout::CreateInfo& createInfo);
		~VulkanDescriptorSetLayout();
		const DescriptorSetLayout::Binding& GetBinding(size_t bindingIndex) const;
		VkDescriptorSetLayout GetInternalLayout() const;
	private:
		VkDescriptorSetLayout descriptorSetLayout;
		DescriptorSetLayout::Binding* bindings = nullptr;
		size_t bindingCount = 0;
	};
}
