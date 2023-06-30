#pragma once

#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class DescriptorSetLayout {
		public:
			struct CreateInfo {
				struct Binding {
					uint32_t bindingId;
					uint32_t count;
					BindingType type;
					ShaderStageBit stages;
				};

				Binding* bindings;
				uint32_t bindingCount;
			};
		};
	}
}
