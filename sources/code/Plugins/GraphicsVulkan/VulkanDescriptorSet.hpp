#pragma once

#include <vulkan/vulkan.h>
#include <Common/Graphics/DescriptorSet.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanDescriptorSet : public DescriptorSet {
		public:
			VulkanDescriptorSet(DescriptorSet::CreateInfo& createInfo);
			~VulkanDescriptorSet();

			virtual void ChangeBindings(DescriptorSet::Binding* bindings, uint32_t bindingCount, uint32_t bindingOffset = 0) override;
			virtual VkDescriptorSet GetDescriptorSet();
		private:
			VkDescriptorSet descriptorSet = nullptr;
			class VulkanDescriptorSetLayout* layout = nullptr;
		};
	}
}
