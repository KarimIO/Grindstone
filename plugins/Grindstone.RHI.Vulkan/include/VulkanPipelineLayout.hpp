#pragma once

#include <Common/Graphics/PipelineLayout.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

namespace Grindstone::GraphicsAPI::Vulkan {
	class PipelineLayout : public Grindstone::GraphicsAPI::PipelineLayout {
	public:
		PipelineLayout(const Grindstone::GraphicsAPI::PipelineLayout::CreateInfo& createInfo);
		~PipelineLayout();
		VkPipelineLayout GetPipelineLayout() const;
	private:
		VkPipelineLayout pipelineLayout = nullptr;
	};
}
