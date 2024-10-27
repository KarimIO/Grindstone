#pragma once

#include <vulkan/vulkan.h>
#include <Common/Graphics/DescriptorSet.hpp>

namespace Grindstone::GraphicsAPI::Vulkan {
	class DescriptorSet : public Grindstone::GraphicsAPI::DescriptorSet {
	public:
		DescriptorSet(const CreateInfo& createInfo);
		~DescriptorSet();

		virtual void ChangeBindings(DescriptorSet::Binding* bindings, uint32_t bindingCount, uint32_t bindingOffset = 0) override;
		virtual VkDescriptorSet GetDescriptorSet() const;
	private:
		VkDescriptorSet descriptorSet = nullptr;
		class DescriptorSetLayout* layout = nullptr;
	};
}
