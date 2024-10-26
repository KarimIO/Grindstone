#pragma once

#include <Common/Graphics/DescriptorSetLayout.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class DescriptorSetLayout : public Grindstone::GraphicsAPI::DescriptorSetLayout {
	public:
		DescriptorSetLayout(const CreateInfo& createInfo);
		~DescriptorSetLayout();
		const DescriptorSetLayout::Binding& GetBinding(size_t bindingIndex) const;
		VkDescriptorSetLayout GetInternalLayout() const;
	private:
		VkDescriptorSetLayout descriptorSetLayout;
		DescriptorSetLayout::Binding* bindings = nullptr;
		size_t bindingCount = 0;
	};
}
