#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include "VulkanRenderPass.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanDescriptorSetLayout.hpp"
#include "VulkanGraphicsPipeline.hpp"

using namespace Grindstone::GraphicsAPI;

static VkShaderStageFlagBits TranslateShaderStageToVulkanBit(ShaderStage stage) {
	switch (stage) {
	default:
	case ShaderStage::Vertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case ShaderStage::Fragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case ShaderStage::Geometry:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	case ShaderStage::TesselationControl:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case ShaderStage::TesselationEvaluation:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case ShaderStage::Compute:
		return VK_SHADER_STAGE_COMPUTE_BIT;
	}
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(GraphicsPipeline::CreateInfo& createInfo) {
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages(createInfo.shaderStageCreateInfoCount);

	for (uint32_t i = 0; i < createInfo.shaderStageCreateInfoCount; ++i) {
		CreateShaderModule(createInfo.shaderStageCreateInfos[i], shaderStages[i]);
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = createInfo.vertexBindingsCount;
	vertexInputInfo.pVertexBindingDescriptions = new VkVertexInputBindingDescription[createInfo.vertexBindingsCount];
	uint32_t vertexAttributeDescriptorCount = 0;
	for (uint32_t i = 0; i < createInfo.vertexBindingsCount; ++i) {
		auto& vb = createInfo.vertexBindings[i];
		VkVertexInputBindingDescription& vbd = (VkVertexInputBindingDescription&)vertexInputInfo.pVertexBindingDescriptions[i];
		vbd = {};
		vbd.binding = i;
		vbd.inputRate = vb.elementRate ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
		vbd.stride = vb.stride;
		vertexAttributeDescriptorCount += vb.attributeCount;
	}

	vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptorCount;
	std::vector<VkVertexInputAttributeDescription> vertexAttribDescriptors(vertexAttributeDescriptorCount);
	vertexAttributeDescriptorCount = 0;

	// TODO: We should reinvestigate setting the binding and locations
	for (uint32_t i = 0; i < createInfo.vertexBindingsCount; ++i) {
		auto& vb = createInfo.vertexBindings[i];

		for (uint32_t j = 0; j < vb.attributeCount; ++j) {
			auto& va = vb.attributes[j];
			VkVertexInputAttributeDescription& vad = vertexAttribDescriptors[vertexAttributeDescriptorCount++];
			vad.binding = i;
			vad.location = va.location;
			vad.format = TranslateVertexFormatsToVulkan(va.format);
			vad.offset = va.offset;
		}
	}

	vertexInputInfo.pVertexAttributeDescriptions = vertexAttribDescriptors.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	static VkPrimitiveTopology primTypes[] = {
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
		VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
	};
	inputAssembly.topology = primTypes[static_cast<size_t>(createInfo.primitiveType)];
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = createInfo.width;
	viewport.height = createInfo.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { createInfo.scissorX, createInfo.scissorY };
	scissor.extent = { createInfo.scissorW, createInfo.scissorH };

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	static VkPolygonMode polygonModes[] = {
		VK_POLYGON_MODE_POINT,
		VK_POLYGON_MODE_LINE,
		VK_POLYGON_MODE_FILL
	};

	rasterizer.polygonMode = polygonModes[static_cast<size_t>(createInfo.polygonFillMode)];

	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = TranslateCullModeToVulkan(createInfo.cullMode);
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	if (createInfo.isDepthBiasEnabled) {
		rasterizer.depthBiasEnable = VK_TRUE;
		rasterizer.depthBiasConstantFactor = 1.25f;
		rasterizer.depthBiasSlopeFactor = 1.75f;
		rasterizer.depthBiasClamp = 0.0f;
	}
	else {
		rasterizer.depthBiasEnable = VK_FALSE;
	}

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = createInfo.isDepthTestEnabled;
	depthStencil.depthWriteEnable = createInfo.isDepthWriteEnabled;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = createInfo.isStencilEnabled;

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(createInfo.colorAttachmentCount);
	for (size_t i = 0; i < createInfo.colorAttachmentCount; ++i) {
		VkPipelineColorBlendAttachmentState& attachment = colorBlendAttachments[i];
		colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		switch (createInfo.blendMode) {
		default:
			attachment.blendEnable = VK_FALSE;
			break;
		case BlendMode::Additive:
			attachment.blendEnable = VK_TRUE;
			attachment.colorBlendOp = VK_BLEND_OP_ADD;
			attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
			attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			attachment.alphaBlendOp = VK_BLEND_OP_ADD;
			attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			break;
		case BlendMode::AdditiveAlpha:
			attachment.blendEnable = VK_TRUE;
			attachment.colorBlendOp = VK_BLEND_OP_ADD;
			attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			attachment.alphaBlendOp = VK_BLEND_OP_ADD;
			attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		}
	}

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = createInfo.colorAttachmentCount;
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

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
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(VulkanCore::Get().GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create pipeline layout!");
	}

	VulkanRenderPass* renderPass = static_cast<VulkanRenderPass*>(createInfo.renderPass);

	std::vector<VkDynamicState> dynamicStates;
	if (createInfo.hasDynamicViewport) {
		dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	}
	if (createInfo.hasDynamicScissor) {
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

	if (vkCreateGraphicsPipelines(VulkanCore::Get().GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create graphics pipeline!");
	}

	if (createInfo.debugName != nullptr) {
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_PIPELINE, graphicsPipeline, createInfo.debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Graphics Pipeline!");
	}

	for (uint32_t i = 0; i < createInfo.shaderStageCreateInfoCount; ++i) {
		vkDestroyShaderModule(VulkanCore::Get().GetDevice(), shaderStages[i].module, nullptr);
	}
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
	VkDevice device = VulkanCore::Get().GetDevice();
	if (graphicsPipeline != nullptr) {
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
	}

	if (pipelineLayout != nullptr) {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	}
}

VkPipeline VulkanGraphicsPipeline::GetGraphicsPipeline() const {
	return graphicsPipeline;
}

VkPipelineLayout VulkanGraphicsPipeline::GetGraphicsPipelineLayout() const {
	return pipelineLayout;
}

void VulkanGraphicsPipeline::Recreate(CreateInfo& createInfo) {
}

void VulkanGraphicsPipeline::CreateShaderModule(ShaderStageCreateInfo & createInfo, VkPipelineShaderStageCreateInfo &out) {
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = createInfo.size;
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(createInfo.content);

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(VulkanCore::Get().GetDevice(), &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create shader module!");
	}

	out = {};
	out.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	out.stage = TranslateShaderStageToVulkanBit(createInfo.type);
	out.module = shaderModule;
	out.pName = "main";
}
