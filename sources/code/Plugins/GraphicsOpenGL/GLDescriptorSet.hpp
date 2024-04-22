#pragma once

#include <Common/Graphics/DescriptorSet.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLDescriptorSet : public DescriptorSet {
		public:
			GLDescriptorSet(CreateInfo& createInfo);
			virtual ~GLDescriptorSet();

			// Inherited via DescriptorSet
			virtual void ChangeBindings(Binding* bindings, uint32_t bindingCount, uint32_t bindingOffset = 0) override;
		};
	}
}
