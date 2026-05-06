#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <xkeycheck.h>

#include <Common/Hash.hpp>
#include "DescriptorSetLayout.hpp"
#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	class RenderPass;
	class DescriptorSetLayout;

	/*! PipelineLayouts define how Descriptors can be bound to Pipelines.
	*/
	class PipelineLayout {
	public:
		struct CreateInfo {
			const char* debugName;
			const DescriptorSetLayout* const* descriptorSetLayouts;
			uint32_t descriptorSetLayoutCount;
		};

		std::vector<const DescriptorSetLayout*> descriptorSetLayouts;
	};
}

namespace std {
	template<>
	struct hash<Grindstone::GraphicsAPI::PipelineLayout::CreateInfo> {
		std::size_t operator()(const Grindstone::GraphicsAPI::PipelineLayout::CreateInfo& pipelineLayout) const noexcept {
			size_t result = std::hash<uint32_t>{}(pipelineLayout.descriptorSetLayoutCount);
			for (uint8_t i = 0; i < pipelineLayout.descriptorSetLayoutCount; ++i) {
				result ^= std::hash<Grindstone::GraphicsAPI::DescriptorSetLayout>{}(*pipelineLayout.descriptorSetLayouts[i]);
			}

			return result;
		}
	};
}
