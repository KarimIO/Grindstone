#include "VulkanComputePipeline.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanDescriptorSetLayout.hpp"

#include <vulkan/vulkan.h>

using namespace Grindstone::GraphicsAPI;

VulkanComputePipeline::VulkanComputePipeline(ComputePipeline::CreateInfo& createInfo) {
	auto device = VulkanCore::Get().GetDevice();

	VkShaderModule computeShaderModule;
	{
		VkShaderModuleCreateInfo computeShaderCreateInfo{};
		computeShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		computeShaderCreateInfo.codeSize = static_cast<size_t>(createInfo.shaderSize);
		computeShaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(createInfo.shaderContent);

		if (vkCreateShaderModule(device, &computeShaderCreateInfo, nullptr, &computeShaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
	}

	{
		std::vector<VkDescriptorSetLayout> layouts;
		layouts.reserve(createInfo.descriptorSetLayoutCount);

		for (uint32_t i = 0; i < createInfo.descriptorSetLayoutCount; ++i) {
			VulkanDescriptorSetLayout* uboBinding = static_cast<VulkanDescriptorSetLayout*>(createInfo.descriptorSetLayouts[i]);
			VkDescriptorSetLayout ubb = uboBinding->GetInternalLayout();
			layouts.push_back(ubb);
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		pipelineLayoutInfo.pSetLayouts = layouts.data();

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline layout!");
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
			throw std::runtime_error("failed to create compute pipeline!");
		}
	}

	if (createInfo.debugName != nullptr) {
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_PIPELINE, computePipeline, createInfo.debugName);
	}

	vkDestroyShaderModule(device, computeShaderModule, nullptr);
}

VulkanComputePipeline::~VulkanComputePipeline() {
	VkDevice device = VulkanCore::Get().GetDevice();
	if (computePipeline != nullptr) {
		vkDestroyPipeline(device, computePipeline, nullptr);
	}

	if (pipelineLayout != nullptr) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}
}

VkPipeline VulkanComputePipeline::GetComputePipeline() const {
	return computePipeline;
}

VkPipelineLayout VulkanComputePipeline::GetComputePipelineLayout() const {
	return pipelineLayout;
}

void VulkanComputePipeline::Recreate(CreateInfo& createInfo) {
}
