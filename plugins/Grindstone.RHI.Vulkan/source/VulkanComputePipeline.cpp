#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include <Grindstone.RHI.Vulkan/include/VulkanRenderPass.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanCore.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanFormat.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanBuffer.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanDescriptorSetLayout.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanComputePipeline.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanPipelineLayout.hpp>

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
		VkPipelineShaderStageCreateInfo computeShaderStageInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			.module = computeShaderModule,
			.pName = "main"
		};

		VkComputePipelineCreateInfo pipelineInfo{
			.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
			.stage = computeShaderStageInfo,
			.layout = static_cast<Vulkan::PipelineLayout*>(createInfo.pipelineLayout)->GetPipelineLayout()
		};

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
}

VkPipeline Vulkan::ComputePipeline::GetComputePipeline() const {
	return computePipeline;
}

void Vulkan::ComputePipeline::Recreate([[maybe_unused]] const CreateInfo& createInfo) {
}
