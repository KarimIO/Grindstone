#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include <Grindstone.RHI.Vulkan/include/VulkanRenderPass.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCore.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanFormat.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanBuffer.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanDescriptorSetLayout.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanPipelineLayout.hpp>

namespace GraphicsAPI = Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;


Vulkan::PipelineLayout::PipelineLayout(const Grindstone::GraphicsAPI::PipelineLayout::CreateInfo& createInfo) {
	std::vector<VkDescriptorSetLayout> layouts;
	layouts.reserve(createInfo.descriptorSetLayoutCount);

	const Vulkan::DescriptorSetLayout* const* descriptorSetLayouts = reinterpret_cast<const Vulkan::DescriptorSetLayout* const*>(createInfo.descriptorSetLayouts);
	this->descriptorSetLayouts.resize(createInfo.descriptorSetLayoutCount);
	for (uint32_t i = 0; i < createInfo.descriptorSetLayoutCount; ++i) {
		this->descriptorSetLayouts[i] = createInfo.descriptorSetLayouts[i];

		const Vulkan::DescriptorSetLayout* descriptorSetLayout = descriptorSetLayouts[i];
		VkDescriptorSetLayout vkDescriptorSetLayout = descriptorSetLayout->GetInternalLayout();
		layouts.push_back(vkDescriptorSetLayout);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data(),
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr
	};

	if (vkCreatePipelineLayout(Vulkan::Core::Get().GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		if (createInfo.debugName != nullptr) {
			GPRINT_FATAL_V(LogSource::GraphicsAPI, "Failed to create pipeline layout '{}'!", createInfo.debugName);
		}
		else {
			GPRINT_FATAL(LogSource::GraphicsAPI, "Failed to create unnamed pipeline layout!");
		}
		return;
	}

	if (createInfo.debugName != nullptr) {
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_PIPELINE_LAYOUT, pipelineLayout, createInfo.debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Graphics Pipeline!");
		return;
	}
}

Vulkan::PipelineLayout::~PipelineLayout() {
	VkDevice device = Vulkan::Core::Get().GetDevice();
	if (pipelineLayout != nullptr) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}
}

VkPipelineLayout Vulkan::PipelineLayout::GetPipelineLayout() const {
	return pipelineLayout;
}
