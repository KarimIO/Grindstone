#include "VulkanRenderPass.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanRenderPass::VulkanRenderPass(RenderPass::CreateInfo& ci) : width(ci.width), height(ci.height) {
			uint32_t total = ci.colorFormatCount;
			total += (ci.depthFormat != DepthFormat::None) ? 1 : 0;

			std::vector<VkAttachmentDescription> attachmentDescs(total);
			std::vector<VkAttachmentReference> attachmentRefs(ci.colorFormatCount);
			
			for (uint32_t i = 0; i < ci.colorFormatCount; ++i) {
				VkAttachmentDescription &colorAttachment = attachmentDescs[i];
				uint8_t channels;
				colorAttachment.format = TranslateColorFormatToVulkan(ci.colorFormats[i], channels);
				colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				colorAttachment.flags = 0;

				attachmentRefs[i].attachment = i;
				attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			VkAttachmentReference *depthAttachmentRefPtr = nullptr;
			VkAttachmentReference depthAttachmentRef = {};
			if (ci.depthFormat != DepthFormat::None) {
				VkAttachmentDescription &depthAttachment = attachmentDescs[ci.colorFormatCount];
				depthAttachment.format = TranslateDepthFormatToVulkan(ci.depthFormat);
				depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachment.flags = 0;

				depthAttachmentRef.attachment = ci.colorFormatCount;
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachmentRefPtr = &depthAttachmentRef;
			}

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = attachmentRefs.size();
			subpass.pColorAttachments = attachmentRefs.data();
			subpass.pDepthStencilAttachment = depthAttachmentRefPtr;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = attachmentDescs.size();
			renderPassInfo.pAttachments = attachmentDescs.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(VulkanCore::Get().GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
				throw std::runtime_error("failed to create render pass!");
			}
		}

		VulkanRenderPass::~VulkanRenderPass() {
			vkDestroyRenderPass(VulkanCore::Get().GetDevice(), renderPass, nullptr);
		}

		VkRenderPass VulkanRenderPass::GetRenderPassHandle() {
			return renderPass;
		}

		uint32_t VulkanRenderPass::GetWidth() {
			return width;
		}

		uint32_t VulkanRenderPass::GetHeight() {
			return height;
		}
	}
}
