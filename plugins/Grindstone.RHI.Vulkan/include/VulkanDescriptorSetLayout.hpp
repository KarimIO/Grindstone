#pragma once

#include <Common/Graphics/DescriptorSetLayout.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class DescriptorSetLayout : public Grindstone::GraphicsAPI::DescriptorSetLayout {
	public:
		DescriptorSetLayout(const CreateInfo& createInfo);
		~DescriptorSetLayout();
		CreateInfo GetCreateInfo() const;
		const DescriptorSetLayout::Binding& GetBinding(size_t bindingIndex) const;
		VkDescriptorSetLayout GetInternalLayout() const;
	private:
		CreateInfo createInfo;
		VkDescriptorSetLayout descriptorSetLayout;
	};
}
