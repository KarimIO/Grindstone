#pragma once

#include <vector>

#include "Formats.hpp"

namespace Grindstone:: GraphicsAPI {
	/*! A descriptor is a reference to a data to be passed to a Pipeline. A
		DescriptorSetLayout is a template that a DescriptorSet can reference to ensure
		they match. They are used upon creation of a Pipeline to ensure the
		correct DescriptorSet formats are passed to them.
	*/
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

	protected:
		std::vector<Binding> bindings;
		size_t bindingCount = 0;

	};
}
