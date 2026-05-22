#pragma once

#include <Common/Graphics/ComputePipeline.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class ComputePipeline : public Grindstone::GraphicsAPI::ComputePipeline {
	public:
		ComputePipeline(const CreateInfo& createInfo);
		~ComputePipeline();
		VkPipeline GetComputePipeline() const;
	public:
		virtual void Bind() {};
		virtual void Recreate(const CreateInfo& createInfo) override;
	private:
		Grindstone::GraphicsAPI::PipelineLayout* pipelineLayout = nullptr;
		VkPipeline computePipeline = nullptr;
	};
}
