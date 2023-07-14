#pragma once

#include <Common/Graphics/ComputePipeline.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		class VulkanComputePipeline : public ComputePipeline {
		public:
			VulkanComputePipeline(ComputePipeline::CreateInfo& createInfo);
			~VulkanComputePipeline();
			VkPipeline GetComputePipeline();
			VkPipelineLayout GetComputePipelineLayout();
		public:
			virtual void Bind() {};
			virtual void Recreate(ComputePipeline::CreateInfo& createInfo) override;
		private:
			VkPipelineLayout pipelineLayout = nullptr;
			VkPipeline computePipeline = nullptr;
		};
	}
}
