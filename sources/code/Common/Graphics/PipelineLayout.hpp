#pragma once

#include <iostream>
#include <xhash>
#include <xkeycheck.h>
#include <vector>

#include <Common/Hash.hpp>

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
			DescriptorSetLayoutRef* descriptorSetLayouts;
			uint32_t descriptorSetLayoutCount;

			bool operator==(const PipelineLayout::CreateInfo& o) const {
				if (descriptorSetLayoutCount != o.descriptorSetLayoutCount) {
					return false;
				}

				for (uint32_t i = 0; i < descriptorSetLayoutCount; ++i) {
					DescriptorSetLayoutRef& a = descriptorSetLayouts[i];
					DescriptorSetLayoutRef& b = o.descriptorSetLayouts[i];
					if (a != b) {
						return false;
					}
				}

				return true;
			}

			bool operator!=(const PipelineLayout::CreateInfo& o) const {
				return !(*this == o);
			}
		};

		std::vector<DescriptorSetLayoutRef> descriptorSetLayouts;
	};
}

namespace std {
	template<>
	struct hash<Grindstone::GraphicsAPI::PipelineLayout::CreateInfo> {
		std::size_t operator()(const Grindstone::GraphicsAPI::PipelineLayout::CreateInfo& pipelineLayout) const noexcept {
			size_t result = std::hash<uint32_t>{}(pipelineLayout.descriptorSetLayoutCount);
			for (uint8_t i = 0; i < pipelineLayout.descriptorSetLayoutCount; ++i) {
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
