#include "VkGraphicsPipeline.hpp"
#include "VkRenderPass.hpp"
#include <iostream>
#include <vector>

#include "VkVertexBuffer.hpp"
#include "VkBufferCommon.hpp"
#include "VkUniformBuffer.hpp"
#include "VkTexture.hpp"
#include <cstring>

VkShaderModule vkGraphicsPipeline::createShaderModule(ShaderStageCreateInfo createInfo) {
	size_t codeLength = createInfo.size;

	VkShaderModuleCreateInfo shaderCreateInfo = {};
	shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCreateInfo.codeSize = codeLength;

	std::vector<uint32_t> codeAligned(createInfo.size / 4 + 1);
	std::memcpy(codeAligned.data(), createInfo.content, codeLength);

	shaderCreateInfo.pCode = codeAligned.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(*device, &shaderCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("VULKAN: Failed to create shader module!");
	}

	return shaderModule;
}

vkGraphicsPipeline::vkGraphicsPipeline(VkDevice *dev, GraphicsPipelineCreateInfo createInfo) {
	device = dev;

	size_t numStages = createInfo.shaderStageCreateInfoCount;

	std::vector<VkShaderModule> shaderModules;
	shaderModules.resize(numStages);
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	shaderStages.reserve(numStages);
	for (int i = 0; i < numStages; i++) {
		shaderModules[i] = createShaderModule(createInfo.shaderStageCreateInfos[i]);

		VkPipelineShaderStageCreateInfo shaderStageInfo = {};
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.module = shaderModules[i];

		switch (createInfo.shaderStageCreateInfos[i].type) {
		case SHADER_FRAGMENT:
			shaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case SHADER_TESS_EVALUATION:
			shaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			break;
		case SHADER_TESS_CONTROL:
			shaderStageInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			break;
		case SHADER_GEOMETRY:
			shaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			break;
		default:
		case SHADER_VERTEX:
			shaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		}

		shaderStageInfo.pName = "main";

		shaderStages.push_back(shaderStageInfo);
	}

	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(createInfo.attributesCount);
	for (int i = 0; i < attributeDescriptions.size(); i++) {
		attributeDescriptions[i].binding = createInfo.attributes[i].binding;
		attributeDescriptions[i].location = createInfo.attributes[i].location;
		attributeDescriptions[i].format = TranslateFormat(createInfo.attributes[i].format);
		attributeDescriptions[i].offset = createInfo.attributes[i].offset;
	}

	std::vector<VkVertexInputBindingDescription> bindingDescriptions(createInfo.bindingsCount);
	for (int i = 0; i < bindingDescriptions.size(); i++) {
		bindingDescriptions[i].binding = createInfo.bindings[i].binding;
		bindingDescriptions[i].inputRate = createInfo.bindings[i].elementRate ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
		bindingDescriptions[i].stride = createInfo.bindings[i].stride;
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	/*multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional*/

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
	descriptorSetLayouts.reserve(createInfo.uniformBufferBindingCount + createInfo.textureBindingCount);
	for (uint32_t i = 0; i < createInfo.uniformBufferBindingCount; i++) {
		vkUniformBufferBinding *vkub = (vkUniformBufferBinding *)(&createInfo.uniformBufferBindings[i]);
		descriptorSetLayouts.push_back(*vkub->getLayout());
	}
	
	for (uint32_t i = 0; i < createInfo.textureBindingCount; i++) {
		vkTextureBindingLayout *vktb = (vkTextureBindingLayout *)(&createInfo.textureBindings[i]);
		descriptorSetLayouts.push_back(*vktb->getLayout());
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	//pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

	if (vkCreatePipelineLayout(*device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.minDepthBounds = 0.0f; // Optional
	depthStencilInfo.maxDepthBounds = 1.0f; // Optional
	depthStencilInfo.stencilTestEnable = VK_FALSE;
	depthStencilInfo.front = {}; // Optional
	depthStencilInfo.back = {}; // Optional
	depthStencilInfo.flags = 0;
	depthStencilInfo.pNext = nullptr; 

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencilInfo; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	//pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipelineLayout;

	vkRenderPass *rp = (vkRenderPass *)createInfo.renderPass;
	pipelineInfo.renderPass = *rp->GetRenderPass();
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	//pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(*device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		std::cerr << "failed to create graphics pipeline!\n";
	}

	for (int i = 0; i < numStages; i++) {
		vkDestroyShaderModule(*device, shaderModules[i], nullptr);
	}
}

VkPipeline *vkGraphicsPipeline::GetGraphicsPipeline() {
	return &graphicsPipeline;
}

VkPipelineLayout * vkGraphicsPipeline::GetPipelineLayout() {
	return &pipelineLayout;
}

vkGraphicsPipeline::~vkGraphicsPipeline() {
	vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
	vkDestroyPipeline(*device, graphicsPipeline, nullptr);
}
