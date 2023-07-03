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
				const char* debugName = nullptr;
				DescriptorSetLayout* layout = nullptr;
				Binding* bindings = nullptr;
				uint32_t bindingCount = 0;
			};
		public:
			virtual void ChangeBindings(Binding* bindings, uint32_t bindingCount) = 0;
		};
	}
}
