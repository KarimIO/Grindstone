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

		std::vector<Binding> bindings;
		size_t bindingCount = 0;

	};
}

namespace std {
	template<>
	struct hash<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding> {
		std::size_t operator()(const Grindstone::GraphicsAPI::DescriptorSetLayout::Binding& binding) const noexcept {
			size_t result = std::hash<uint32_t>{}(binding.bindingId);
			result ^= std::hash<uint32_t>{}(binding.count);
			result ^= std::hash<uint8_t>{}(static_cast<uint8_t>(binding.stages));
			result ^= std::hash<uint8_t>{}(static_cast<uint8_t>(binding.type));

			return result;
		}
	};

	template<>
	struct hash<Grindstone::GraphicsAPI::DescriptorSetLayout> {
		std::size_t operator()(const Grindstone::GraphicsAPI::DescriptorSetLayout& descriptorSetLayout) const noexcept {
			size_t result = std::hash<size_t>{}(descriptorSetLayout.bindingCount);
			for (uint8_t i = 0; i < descriptorSetLayout.bindingCount; ++i) {
				result ^= std::hash<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding>{}(descriptorSetLayout.bindings[i]);
			}

			return result;
		}
	};

	template<>
	struct hash<Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo> {
		std::size_t operator()(const Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo& createInfo) const noexcept {
			size_t result = std::hash<size_t>{}(createInfo.bindingCount);
			for (uint8_t i = 0; i < createInfo.bindingCount; ++i) {
				result ^= std::hash<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding>{}(createInfo.bindings[i]);
			}

			return result;
		}
	};
}
