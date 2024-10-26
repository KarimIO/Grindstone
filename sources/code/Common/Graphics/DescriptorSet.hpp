#pragma once

#include "DescriptorSetLayout.hpp"
#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	class DescriptorSet {
	public:
		struct Binding {
			void* itemPtr;
			uint32_t count = 1;

			Binding() = default;
			Binding(const Binding& binding) = default;
			Binding(Binding&& binding) = default;
			Binding& operator=(const Binding& binding) = default;
			Binding(void* itemPtr) : itemPtr(itemPtr), count(1) {}
			Binding(void* itemPtr, uint32_t count) : itemPtr(itemPtr), count(count) {}
		};

		struct CreateInfo {
			const char* debugName = nullptr;
			DescriptorSetLayout* layout = nullptr;
			Binding* bindings = nullptr;
			uint32_t bindingCount = 0;
		};
	public:
		virtual void ChangeBindings(DescriptorSet::Binding* bindings, uint32_t bindingCount, uint32_t bindingOffset = 0) = 0;
	};
}
