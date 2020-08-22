#include "DirectX12RenderPass.hpp"
#include "DirectX12GraphicsWrapper.hpp"
#include "DirectX12Format.hpp"
#include <d3d12.h>

namespace Grindstone {
	namespace GraphicsAPI {
		DirectX12RenderPass::DirectX12RenderPass(RenderPassCreateInfo ci) : width_(ci.m_width), height_(ci.m_height) {
			uint32_t total = ci.m_colorClearCount;
			total += (ci.m_depthFormat != DepthFormat::None) ? 1 : 0;

			VkAttachmentDescription *attachmentDescs = new VkAttachmentDescription[total];
			VkAttachmentReference *attachmentsRef = new VkAttachmentReference[ci.m_colorClearCount];
			
			for (uint32_t i = 0; i < ci.m_colorClearCount; ++i) {
				VkAttachmentDescription &colorAttachment = attachmentDescs[i];
				uint8_t channels;
				colorAttachment.format = TranslateColorFormatToDirectX12(ci.m_colorFormats[i], channels);
				colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				colorAttachment.flags = 0;

				attachmentsRef[i].attachment = i;
				attachmentsRef[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			VkAttachmentReference *depthAttachmentRefPtr = nullptr;
			VkAttachmentReference depthAttachmentRef = {};
			if (ci.m_depthFormat != DepthFormat::None) {
				VkAttachmentDescription &depthAttachment = attachmentDescs[ci.m_colorClearCount];
				depthAttachment.format = TranslateDepthFormatToDirectX12(ci.m_depthFormat);
				depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachment.flags = 0;

				depthAttachmentRef.attachment = ci.m_colorClearCount;
				depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthAttachmentRefPtr = &depthAttachmentRef;
			}

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = ci.m_colorClearCount;
			subpass.pColorAttachments = attachmentsRef;
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
			renderPassInfo.attachmentCount = total;
			renderPassInfo.pAttachments = attachmentDescs;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(DirectX12GraphicsWrapper::get().getDevice(), &renderPassInfo, nullptr, &render_pass_) != VK_SUCCESS) {
				throw std::runtime_error("failed to create render pass!");
			}
		}

		VkRenderPass DirectX12RenderPass::getRenderPassHandle() {
			return render_pass_;
		}

		uint32_t DirectX12RenderPass::getWidth() {
			return width_;
		}

		uint32_t DirectX12RenderPass::getHeight() {
			return height_;
		}
	}
}
