#pragma once

#include "DescriptorSetLayout.hpp"
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class DescriptorSet {
		public:
			struct Binding {
				void* itemPtr;
				BindingType bindingType;
				uint32_t bindingIndex;
				uint32_t count = 1;
			};

			struct CreateInfo {
				DescriptorSetLayout* layout;
				Binding* bindings;
				uint32_t bindingCount;
			};
		public:
			virtual void ChangeBindings(Binding* bindings, uint32_t bindingCount) = 0;
		};
	}
}
