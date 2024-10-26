#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include "VulkanRenderPass.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanComputePipeline.hpp"

namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::ComputePipeline::ComputePipeline(const CreateInfo& createInfo) {
	VkDevice device = Vulkan::Core::Get().GetDevice();

	VkShaderModule computeShaderModule;
	{
		VkShaderModuleCreateInfo computeShaderCreateInfo{};
		computeShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		computeShaderCreateInfo.codeSize = static_cast<size_t>(createInfo.shaderSize);
		computeShaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(createInfo.shaderContent);

		if (vkCreateShaderModule(device, &computeShaderCreateInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "Failed to create shader module!");
		}
	}

	{
		std::vector<VkDescriptorSetLayout> layouts;
		layouts.reserve(createInfo.descriptorSetLayoutCount);

		for (uint32_t i = 0; i < createInfo.descriptorSetLayoutCount; ++i) {
			Vulkan::DescriptorSetLayout* uboBinding = static_cast<Vulkan::DescriptorSetLayout*>(createInfo.descriptorSetLayouts[i]);
			VkDescriptorSetLayout ubb = uboBinding->GetInternalLayout();
			layouts.push_back(ubb);
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutInfo.pSetLayouts = layouts.data();

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create compute pipeline layout!");
		}
	}

	{
		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = computeShaderModule;
		computeShaderStageInfo.pName = "main";

		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.stage = computeShaderStageInfo;

		if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create compute pipeline!");
		}
	}

	if (createInfo.debugName != nullptr) {
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_PIPELINE, computePipeline, createInfo.debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Compute Pipeline!");
	}

	vkDestroyShaderModule(device, computeShaderModule, nullptr);
}

Vulkan::ComputePipeline::~ComputePipeline() {
	VkDevice device = Vulkan::Core::Get().GetDevice();
	if (computePipeline != nullptr) {
		vkDestroyPipeline(device, computePipeline, nullptr);
	}

	if (pipelineLayout != nullptr) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}
}

VkPipeline Vulkan::ComputePipeline::GetComputePipeline() const {
	return computePipeline;
}

VkPipelineLayout Vulkan::ComputePipeline::GetComputePipelineLayout() const {
	return pipelineLayout;
}

void Vulkan::ComputePipeline::Recreate(const CreateInfo& createInfo) {
}
