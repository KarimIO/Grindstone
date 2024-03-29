#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class DescriptorSetLayout;

		class ComputePipeline {
		public:
			struct CreateInfo {
				const char* debugName;
				const char* shaderFileName;
				const char* shaderContent;
				uint32_t shaderSize;
				DescriptorSetLayout** descriptorSetLayouts;
				uint32_t descriptorSetLayoutCount;
			};

			virtual void Recreate(ComputePipeline::CreateInfo& createInfo) = 0;
		};
	}
}
