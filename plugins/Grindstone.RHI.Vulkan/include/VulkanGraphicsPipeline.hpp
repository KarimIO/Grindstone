#pragma once

#include <Common/Graphics/GraphicsPipeline.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class GraphicsPipeline : public Grindstone::GraphicsAPI::GraphicsPipeline {
	public:
		GraphicsPipeline(const CreateInfo& createInfo);
		~GraphicsPipeline();
		VkPipeline GetGraphicsPipeline() const;
		VkPipelineLayout GetGraphicsPipelineLayout() const;
	public:
		virtual void Bind() {};
	private:
	private:
		VkPipelineLayout pipelineLayout = nullptr;
		VkPipeline graphicsPipeline = nullptr;
	};
}
