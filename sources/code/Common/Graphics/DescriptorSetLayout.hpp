#pragma once

#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class DescriptorSetLayout {
		public:
			struct Binding {
				uint32_t bindingId;
				uint32_t count;
				BindingType type;
				ShaderStageBit stages;
			};

			struct CreateInfo {
				const char* debugName = nullptr;
				Binding* bindings = nullptr;
				uint32_t bindingCount = 0;
			};
		};
	}
}
