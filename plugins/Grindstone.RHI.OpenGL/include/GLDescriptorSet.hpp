#pragma once

#include <Common/Graphics/DescriptorSet.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class DescriptorSet : public Grindstone::GraphicsAPI::DescriptorSet {
	public:
		DescriptorSet(const CreateInfo& createInfo);
		virtual ~DescriptorSet();

		// Inherited via DescriptorSet
		virtual void ChangeBindings(const Binding* bindings, uint32_t bindingCount, uint32_t bindingOffset = 0) override;
	};
}
