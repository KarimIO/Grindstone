#pragma once

#include <vector>
#include <xhash>
#include <xkeycheck.h>

#include <Common/Containers/DynamicArray.hpp>
#include <Common/Containers/ResourceCache.hpp>
#include <Common/Graphics/DescriptorSetLayout.hpp>

namespace Grindstone::GraphicsAPI {
	class RenderPass;
	class DescriptorSetLayout;

	/*! PipelineLayouts define how Descriptors can be bound to Pipelines.
	*/
	class PipelineLayout {
	public:
		struct CreateInfo {
			const char* debugName;
			Grindstone::Containers::DynamicArray<DescriptorSetLayoutRef> descriptorSetLayouts;

			bool operator==(const PipelineLayout::CreateInfo& o) const {
				return descriptorSetLayouts == o.descriptorSetLayouts;
			}

			bool operator!=(const PipelineLayout::CreateInfo& o) const {
				return descriptorSetLayouts != o.descriptorSetLayouts;
			}
		};

		std::vector<DescriptorSetLayoutRef> descriptorSetLayouts;
	};
}

namespace std {
	template<>
	struct hash<Grindstone::GraphicsAPI::PipelineLayout::CreateInfo> {
		std::size_t operator()(const Grindstone::GraphicsAPI::PipelineLayout::CreateInfo& pipelineLayout) const noexcept {
			size_t result = std::hash<size_t>{}(pipelineLayout.descriptorSetLayouts.GetSize());
			for (uint8_t i = 0; i < pipelineLayout.descriptorSetLayouts.GetSize(); ++i) {
				result ^= std::hash<Grindstone::GraphicsAPI::DescriptorSetLayout>{}(*pipelineLayout.descriptorSetLayouts[i].Get());
			}

			return result;
		}
	};
}

namespace Grindstone::GraphicsAPI {
	using PipelineLayoutCache = Grindstone::Containers::ResourceCache<GraphicsAPI::PipelineLayout::CreateInfo, GraphicsAPI::PipelineLayout*>;
	using PipelineLayoutRef = PipelineLayoutCache::Ref;
}
