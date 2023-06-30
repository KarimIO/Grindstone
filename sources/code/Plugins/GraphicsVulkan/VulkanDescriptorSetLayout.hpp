#pragma once

#include <Common/Graphics/DescriptorSetLayout.hpp>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanDescriptorSetLayout : public DescriptorSetLayout {
		public:
			VulkanDescriptorSetLayout(DescriptorSetLayout::CreateInfo& createInfo);
			~VulkanDescriptorSetLayout();
			VkDescriptorSetLayout GetInternalLayout();
		private:
			VkDescriptorSetLayout descriptorSetLayout;
		};
	}
}
