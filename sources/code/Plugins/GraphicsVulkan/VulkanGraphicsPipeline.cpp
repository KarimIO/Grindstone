#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include "VulkanRenderPass.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanBuffer.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanGraphicsPipeline.hpp"

namespace GraphicsAPI = Grindstone::GraphicsAPI;
namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

static VkShaderStageFlagBits TranslateShaderStageToVulkanBit(GraphicsAPI::ShaderStage stage) {
	switch (stage) {
	default:
	case GraphicsAPI::ShaderStage::Vertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case GraphicsAPI::ShaderStage::Fragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case GraphicsAPI::ShaderStage::Geometry:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	case GraphicsAPI::ShaderStage::TesselationControl:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case GraphicsAPI::ShaderStage::TesselationEvaluation:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case GraphicsAPI::ShaderStage::Compute:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	}
}

static void CreateShaderModule(
	GraphicsAPI::GraphicsPipeline::ShaderStageData& createInfo,
	VkPipelineShaderStageCreateInfo& out
) {
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = createInfo.size;
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(createInfo.content);

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(Vulkan::Core::Get().GetDevice(), &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		GPRINT_FATAL(Grindstone::LogSource::GraphicsAPI, "failed to create shader module!");
	}

	out = {};
	out.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	out.stage = TranslateShaderStageToVulkanBit(createInfo.type);
	out.module = shaderModule;
	out.pName = "main";
}

Vulkan::GraphicsPipeline::GraphicsPipeline(const CreateInfo& createInfo) {
	const PipelineData& pipelineData = createInfo.pipelineData;
	const VertexInputLayout& vertexInputLayout = createInfo.vertexInputLayout;

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(pipelineData.shaderStageCreateInfoCount);

	for (uint32_t i = 0; i < pipelineData.shaderStageCreateInfoCount; ++i) {
		CreateShaderModule(pipelineData.shaderStageCreateInfos[i], shaderStages[i]);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	std::vector<VkVertexInputBindingDescription> vkBindings;
	vkBindings.resize(vertexInputLayout.bindings.size());

	for (size_t i = 0; i < vertexInputLayout.bindings.size(); ++i) {
		const auto& bindingSrc = vertexInputLayout.bindings[i];
		VkVertexInputBindingDescription& bindingDst = vkBindings[i];
		bindingDst = {};
		bindingDst.binding = bindingSrc.bindingIndex;
		bindingDst.inputRate = bindingSrc.inputRate == VertexInputRate::Instance
			? VK_VERTEX_INPUT_RATE_INSTANCE
			: VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDst.stride = bindingSrc.stride;
	}

	std::vector<VkVertexInputAttributeDescription> vkAttributes;
	vkAttributes.resize(vertexInputLayout.attributes.size());

	for (size_t i = 0; i < vertexInputLayout.attributes.size(); ++i) {
		const auto& attributeSrc = vertexInputLayout.attributes[i];
		VkVertexInputAttributeDescription& attributeDst = vkAttributes[i];
		attributeDst.binding = attributeSrc.bindingIndex;
		attributeDst.location = attributeSrc.locationIndex;
		attributeDst.format = TranslateFormatToVulkan(attributeSrc.format);
		attributeDst.offset = attributeSrc.byteOffset;
	}

	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vkBindings.size());
	vertexInputInfo.pVertexBindingDescriptions = vkBindings.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vkAttributes.size());
	vertexInputInfo.pVertexAttributeDescriptions = vkAttributes.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = TranslateGeometryTypeToVulkan(pipelineData.primitiveType);
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = pipelineData.width;
	viewport.height = pipelineData.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { pipelineData.scissorX, pipelineData.scissorY };
	scissor.extent = { pipelineData.scissorW, pipelineData.scissorH };

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = pipelineData.isDepthClampEnabled;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = TranslatePolygonModeToVulkan(pipelineData.polygonFillMode);
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = TranslateCullModeToVulkan(pipelineData.cullMode);
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = pipelineData.isDepthBiasEnabled;
	rasterizer.depthBiasConstantFactor = pipelineData.depthBiasConstantFactor;
	rasterizer.depthBiasSlopeFactor = pipelineData.depthBiasSlopeFactor;
	rasterizer.depthBiasClamp = pipelineData.depthBiasClamp;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = pipelineData.isDepthTestEnabled;
	depthStencil.depthWriteEnable = pipelineData.isDepthWriteEnabled;
	depthStencil.depthCompareOp = TranslateCompareOpToVulkan(pipelineData.depthCompareOp);
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = pipelineData.isStencilEnabled;

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(pipelineData.colorAttachmentCount);
	for (uint8_t i = 0; i < pipelineData.colorAttachmentCount; ++i) {
		Grindstone::GraphicsAPI::GraphicsPipeline::AttachmentData& attachmentCreateInfo = pipelineData.colorAttachmentData[i];
		VkPipelineColorBlendAttachmentState& attachment = colorBlendAttachments[i];
		
		colorBlendAttachments[i].colorWriteMask = TranslateColorMaskToVulkan(attachmentCreateInfo.colorMask);

		if (attachmentCreateInfo.blendData.colorOperation == BlendOperation::None && attachmentCreateInfo.blendData.alphaOperation == BlendOperation::None) {
			attachment.blendEnable = VK_FALSE;
		}
		else if (attachmentCreateInfo.blendData.colorOperation == BlendOperation::None || attachmentCreateInfo.blendData.alphaOperation == BlendOperation::None) {
			GPRINT_ERROR(LogSource::GraphicsAPI, "Invalid blendOperation - using none blend with non-none blend");
			attachment.blendEnable = VK_FALSE;
		}
		else {
			attachment.blendEnable = VK_TRUE;
		}

		attachment.colorBlendOp = TranslateBlendOpToVulkan(attachmentCreateInfo.blendData.colorOperation);
		attachment.srcColorBlendFactor = TranslateBlendFactorToVulkan(attachmentCreateInfo.blendData.colorFactorSrc);
		attachment.dstColorBlendFactor = TranslateBlendFactorToVulkan(attachmentCreateInfo.blendData.colorFactorDst);

		attachment.alphaBlendOp = TranslateBlendOpToVulkan(attachmentCreateInfo.blendData.alphaOperation);
		attachment.srcAlphaBlendFactor = TranslateBlendFactorToVulkan(attachmentCreateInfo.blendData.alphaFactorSrc);
		attachment.dstAlphaBlendFactor = TranslateBlendFactorToVulkan(attachmentCreateInfo.blendData.alphaFactorDst);
	}

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = pipelineData.colorAttachmentCount;
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDescriptorSetLayout> layouts;
	layouts.reserve(pipelineData.descriptorSetLayoutCount);


	Vulkan::DescriptorSetLayout** descriptorSetLayouts = reinterpret_cast<Vulkan::DescriptorSetLayout**>(pipelineData.descriptorSetLayouts);

	for (uint32_t i = 0; i < pipelineData.descriptorSetLayoutCount; ++i) {
		Vulkan::DescriptorSetLayout* descriptorSetLayout = descriptorSetLayouts[i];
		VkDescriptorSetLayout vkDescriptorSetLayout = descriptorSetLayout->GetInternalLayout();
		layouts.push_back(vkDescriptorSetLayout);
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
	pipelineLayoutInfo.pSetLayouts = layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(Vulkan::Core::Get().GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create pipeline layout!");
		return;
	}

	Vulkan::RenderPass* renderPass = static_cast<Vulkan::RenderPass*>(pipelineData.renderPass);

	std::vector<VkDynamicState> dynamicStates;
	if (pipelineData.hasDynamicViewport) {
		dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	}
	if (pipelineData.hasDynamicScissor) {
		dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
	}

	VkPipelineDynamicStateCreateInfo dynamicInfo{};
	dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicInfo.pDynamicStates = dynamicStates.data();
	dynamicInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.subpass = 0;
	pipelineInfo.pDynamicState = &dynamicInfo;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.renderPass = (renderPass != nullptr)
		? renderPass->GetRenderPassHandle()
		: nullptr;

	if (vkCreateGraphicsPipelines(Vulkan::Core::Get().GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create graphics pipeline!");
		return;
	}

	if (pipelineData.debugName != nullptr) {
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_PIPELINE, graphicsPipeline, pipelineData.debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Graphics Pipeline!");
		return;
	}

	for (uint32_t i = 0; i < pipelineData.shaderStageCreateInfoCount; ++i) {
		vkDestroyShaderModule(Vulkan::Core::Get().GetDevice(), shaderStages[i].module, nullptr);
	}
}

Vulkan::GraphicsPipeline::~GraphicsPipeline() {
	VkDevice device = Vulkan::Core::Get().GetDevice();
	if (graphicsPipeline != nullptr) {
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
	}

	if (pipelineLayout != nullptr) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}
}

VkPipeline Vulkan::GraphicsPipeline::GetGraphicsPipeline() const {
	return graphicsPipeline;
}

VkPipelineLayout Vulkan::GraphicsPipeline::GetGraphicsPipelineLayout() const {
	return pipelineLayout;
}
