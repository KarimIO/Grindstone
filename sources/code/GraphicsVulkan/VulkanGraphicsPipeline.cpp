#include "VulkanGraphicsPipeline.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanGraphicsWrapper.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanTexture.hpp"

#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VkShaderStageFlagBits TranslateShaderStageToVulkanBit(ShaderStage vk) {
			switch (vk) {
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

		VulkanGraphicsPipeline::VulkanGraphicsPipeline(GraphicsPipelineCreateInfo ci) {
			VkPipelineShaderStageCreateInfo *shaderStages = new VkPipelineShaderStageCreateInfo[ci.shaderStageCreateInfoCount];
			
			for (uint32_t i = 0; i < ci.shaderStageCreateInfoCount; ++i) {
				VkShaderModuleCreateInfo createInfo = {};
				createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				createInfo.codeSize = ci.shaderStageCreateInfos[i].size;
				createInfo.pCode = reinterpret_cast<const uint32_t*>(ci.shaderStageCreateInfos[i].content);
				
				VkShaderModule shaderModule;
				if (vkCreateShaderModule(VulkanGraphicsWrapper::get().getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
					throw std::runtime_error("failed to create shader module!");
				}

				shaderStages[i] = {};
				shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
				shaderStages[i].stage = TranslateShaderStageToVulkanBit(ci.shaderStageCreateInfos[i].type);
				shaderStages[i].module = shaderModule;
				shaderStages[i].pName = "main";
			}
			
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = ci.bindingsCount;
			vertexInputInfo.pVertexBindingDescriptions = new VkVertexInputBindingDescription[ci.bindingsCount];
			for (uint32_t i = 0; i < ci.bindingsCount; ++i) {
				VkVertexInputBindingDescription &vbd = (VkVertexInputBindingDescription &)vertexInputInfo.pVertexBindingDescriptions[i];
				vbd = {};
				vbd.binding = ci.bindings[i].binding;
				vbd.inputRate = ci.bindings[i].elementRate ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
				vbd.stride = ci.bindings[i].stride;
			}
			vertexInputInfo.pVertexAttributeDescriptions = new VkVertexInputAttributeDescription[ci.attributesCount];
			for (uint32_t i = 0; i < ci.attributesCount; ++i) {
				VkVertexInputAttributeDescription &vad = (VkVertexInputAttributeDescription &)vertexInputInfo.pVertexAttributeDescriptions[i];
				vad = {};
				vad.binding = ci.attributes[i].binding;
				vad.location = ci.attributes[i].location;
				vad.format = TranslateVertexFormatsToVulkan(ci.attributes[i].format);
				vad.offset = ci.attributes[i].offset;
			}

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)ci.width;
			viewport.height = (float)ci.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.offset = { ci.scissorX, ci.scissorY };
			scissor.extent = { ci.scissorW, ci.scissorH };

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
			switch (ci.cullMode) {
			case CullMode::None:
				rasterizer.cullMode = VK_CULL_MODE_NONE;
				break;
			case CullMode::Front:
				rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
				break;
			case CullMode::Back:
				rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
				break;
			case CullMode::Both:
				rasterizer.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
				break;
			}
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo multisampling = {};
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineDepthStencilStateCreateInfo depthStencil = {};
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.stencilTestEnable = VK_FALSE;

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

			std::vector<VkDescriptorSetLayout> layouts;
			layouts.reserve(ci.uniformBufferBindingCount + ci.textureBindingCount);

			for (uint32_t i = 0; i < ci.uniformBufferBindingCount; ++i) {
				auto ubb = ((VulkanUniformBufferBinding *)ci.uniformBufferBindings[i])->getDescriptorSetLayout();
				layouts.push_back(ubb);
			}

			for (uint32_t i = 0; i < ci.textureBindingCount; ++i) {
				auto tex = ((VulkanTextureBindingLayout *)ci.textureBindings[i])->getDescriptorSetLayout();
				layouts.push_back(tex);
			}

			VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = layouts.size();
			pipelineLayoutInfo.pSetLayouts = layouts.data();
			pipelineLayoutInfo.pushConstantRangeCount = 0;

			if (vkCreatePipelineLayout(VulkanGraphicsWrapper::get().getDevice(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create pipeline layout!");
			}

			VulkanRenderPass *rp = (VulkanRenderPass *)ci.renderPass;

			VkGraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.stageCount = ci.shaderStageCreateInfoCount;
			pipelineInfo.pStages = shaderStages;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pMultisampleState = &multisampling;
			pipelineInfo.pDepthStencilState = &depthStencil;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.layout = pipeline_layout_;
			pipelineInfo.renderPass = rp->getRenderPassHandle();
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			if (vkCreateGraphicsPipelines(VulkanGraphicsWrapper::get().getDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphics_pipeline_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics pipeline!");
			}

			for (uint32_t i = 0; i < ci.shaderStageCreateInfoCount; ++i) {
				vkDestroyShaderModule(VulkanGraphicsWrapper::get().getDevice(), shaderStages[i].module, nullptr);
			}
		}

		VkPipeline VulkanGraphicsPipeline::getGraphicsPipeline() {
			return graphics_pipeline_;
		}

		VkPipelineLayout VulkanGraphicsPipeline::getGraphicsPipelineLayout() {
			return pipeline_layout_;
		}

		void VulkanGraphicsPipeline::createShaderModule(ShaderStageCreateInfo &ci, VkPipelineShaderStageCreateInfo &out) {
			VkShaderModuleCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = ci.size;
			createInfo.pCode = reinterpret_cast<const uint32_t*>(ci.content);

			VkShaderModule shaderModule;
			if (vkCreateShaderModule(VulkanGraphicsWrapper::get().getDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module!");
			}

			out.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			out.stage = TranslateShaderStageToVulkanBit(ci.type);
			out.module = shaderModule;
			out.pName = "main";
		}
	}
}