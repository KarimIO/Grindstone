#pragma once

#include <cstdint>
#include <vector>

#include <Common/Containers/DynamicArray.hpp>
#include <Common/Containers/ResourceCache.hpp>
#include <Common/Hash.hpp>
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

			bool operator==(const Binding& o) const {
				return
					bindingId == o.bindingId &&
					count == o.count &&
					type == o.type &&
					stages == o.stages;
			}

			bool operator!=(const Binding& o) const {
				return !(*this == o);
			}
		};

		struct CreateInfo {
			const char* debugName = nullptr;
			Grindstone::Containers::DynamicArray<Binding> bindings;

			bool operator==(const CreateInfo& other) const {
				return bindings == other.bindings;
			}

			bool operator!=(const CreateInfo& other) const {
				return bindings != other.bindings;
			}
		};

		Grindstone::Containers::DynamicArray<Binding> bindings;
	};
}

namespace std {
	template<>
	struct hash<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding> {
		std::size_t operator()(const Grindstone::GraphicsAPI::DescriptorSetLayout::Binding& binding) const noexcept {
			size_t result = std::hash<uint32_t>{}(binding.bindingId);
			Grindstone::Hash::Combine<uint32_t>(result, binding.count);
			Grindstone::Hash::Combine<uint8_t>(result, static_cast<uint8_t>(binding.stages));
			Grindstone::Hash::Combine<uint8_t>(result, static_cast<uint8_t>(binding.type));

			return result;
		}
	};

	template<>
	struct hash<Grindstone::GraphicsAPI::DescriptorSetLayout> {
		std::size_t operator()(const Grindstone::GraphicsAPI::DescriptorSetLayout& descriptorSetLayout) const noexcept {
			size_t result = std::hash<size_t>{}(descriptorSetLayout.bindings.GetSize());
			for (uint8_t i = 0; i < descriptorSetLayout.bindings.GetSize(); ++i) {
				result ^= std::hash<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding>{}(descriptorSetLayout.bindings[i]);
			}

			return result;
		}
	};

	template<>
	struct hash<Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo> {
		std::size_t operator()(const Grindstone::GraphicsAPI::DescriptorSetLayout::CreateInfo& createInfo) const noexcept {
			size_t result = std::hash<size_t>{}(createInfo.bindings.GetSize());
			for (uint8_t i = 0; i < createInfo.bindings.GetSize(); ++i) {
				Grindstone::Hash::Combine<Grindstone::GraphicsAPI::DescriptorSetLayout::Binding>(result, createInfo.bindings[i]);
			}

			return result;
		}
	};
}

namespace Grindstone::GraphicsAPI {
	using DescriptorSetLayoutCache = Grindstone::Containers::ResourceCache<GraphicsAPI::DescriptorSetLayout::CreateInfo, GraphicsAPI::DescriptorSetLayout*>;
	using DescriptorSetLayoutRef = DescriptorSetLayoutCache::Ref;
}
