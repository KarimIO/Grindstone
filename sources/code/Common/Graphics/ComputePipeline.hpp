#pragma once

#include <string>
#include <vector>
#include <iostream>

#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	class DescriptorSetLayout;

	/*! Pipelines are a program that runs on the GPU. Compute Pipelines are a variety
		of Pipeline that are not more generic and don't have to create graphical effects.
		Instead, they can be utilized to arbitrarily use the power of the GPU - lots of
		processing units in parallel. They can take in images and buffers that they
		can randomly read and write to.
	*/
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

		virtual void Recreate(const ComputePipeline::CreateInfo& createInfo) = 0;
	};
}
