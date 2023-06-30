#pragma once

#include <vulkan/vulkan.h>
#include <Common/Graphics/DescriptorSet.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanDescriptorSet : public DescriptorSet {
		public:
			VulkanDescriptorSet(DescriptorSet::CreateInfo& createInfo);
			~VulkanDescriptorSet();

			virtual void ChangeBindings(Binding* bindings, uint32_t bindingCount) override;
			VkDescriptorSet GetDescriptorSet();
		private:
			VkDescriptorSet descriptorSet = nullptr;
		};
	}
}
