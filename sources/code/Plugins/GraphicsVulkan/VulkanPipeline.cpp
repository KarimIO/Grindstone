#include "VulkanPipeline.hpp"
#include "VulkanRenderPass.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanUniformBuffer.hpp"
#include "VulkanTexture.hpp"
#include "VulkanDescriptorSetLayout.hpp"

#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VkShaderStageFlagBits TranslateShaderStageToVulkanBit(ShaderStage stage) {
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

		VulkanPipeline::VulkanPipeline(Pipeline::CreateInfo& createInfo) {
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
				VkVertexInputBindingDescription &vbd = (VkVertexInputBindingDescription &)vertexInputInfo.pVertexBindingDescriptions[i];
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
					VkVertexInputAttributeDescription &vad = vertexAttribDescriptors[vertexAttributeDescriptorCount++];
					vad.binding = i;
					vad.location = va.location;
					vad.format = TranslateVertexFormatsToVulkan(va.format);
					vad.offset = va.offset;
				}
			}

			vertexInputInfo.pVertexAttributeDescriptions = vertexAttribDescriptors.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)createInfo.width;
			viewport.height = (float)createInfo.height;
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
			switch (createInfo.cullMode) {
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

			std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(4);

			for (size_t i = 0; i < colorBlendAttachments.size(); ++i) {
				colorBlendAttachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				colorBlendAttachments[i].blendEnable = VK_FALSE;
			}

			VkPipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());
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

			VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
			pipelineLayoutInfo.pSetLayouts = layouts.data();
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;

			if (vkCreatePipelineLayout(VulkanCore::Get().GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
				throw std::runtime_error("failed to create pipeline layout!");
			}

			VulkanRenderPass * renderPass = static_cast<VulkanRenderPass*>(createInfo.renderPass);

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
			pipelineInfo.renderPass = renderPass->GetRenderPassHandle();
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			if (vkCreateGraphicsPipelines(VulkanCore::Get().GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
				throw std::runtime_error("failed to create graphics pipeline!");
			}

			for (uint32_t i = 0; i < createInfo.shaderStageCreateInfoCount; ++i) {
				vkDestroyShaderModule(VulkanCore::Get().GetDevice(), shaderStages[i].module, nullptr);
			}
		}

		VulkanPipeline::~VulkanPipeline() {
			vkDestroyPipeline(VulkanCore::Get().GetDevice(), graphicsPipeline, nullptr);
			vkDestroyPipelineLayout(VulkanCore::Get().GetDevice(), pipelineLayout, nullptr);
		}

		VkPipeline VulkanPipeline::GetGraphicsPipeline() {
			return graphicsPipeline;
		}

		VkPipelineLayout VulkanPipeline::GetGraphicsPipelineLayout() {
			return pipelineLayout;
		}

		void VulkanPipeline::Recreate(CreateInfo& createInfo) {
		}

		void VulkanPipeline::CreateShaderModule(ShaderStageCreateInfo & createInfo, VkPipelineShaderStageCreateInfo &out) {
			VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
			shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderModuleCreateInfo.codeSize = createInfo.size;
			shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(createInfo.content);

			VkShaderModule shaderModule;
			if (vkCreateShaderModule(VulkanCore::Get().GetDevice(), &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
				throw std::runtime_error("failed to create shader module!");
			}

			out = {};
			out.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			out.stage = TranslateShaderStageToVulkanBit(createInfo.type);
			out.module = shaderModule;
			out.pName = "main";
		}
	}
}
