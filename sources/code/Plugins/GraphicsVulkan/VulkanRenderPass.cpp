#include "VulkanRenderPass.hpp"
#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include <vulkan/vulkan.h>

namespace Grindstone {
	namespace GraphicsAPI {
		VulkanRenderPass::VulkanRenderPass(VkRenderPass renderPass, uint32_t width, uint32_t height)
			: renderPass(renderPass), width(width), height(height) {
		}

		VulkanRenderPass::VulkanRenderPass(RenderPass::CreateInfo& createInfo) : width(createInfo.width), height(createInfo.height) {
			if (createInfo.debugName != nullptr) {
				debugName = createInfo.debugName;
			}

			for (uint32_t i = 0; i < createInfo.colorFormatCount; ++i) {
				colorFormats.push_back(createInfo.colorFormats[i]);
			}

			depthFormat = createInfo.depthFormat;

			Create();
		}

		void VulkanRenderPass::Create() {
			uint32_t total = static_cast<uint32_t>(colorFormats.size());
			total += (depthFormat != DepthFormat::None) ? 1 : 0;

			std::vector<VkAttachmentDescription> attachmentDescs(total);
			std::vector<VkAttachmentReference> attachmentRefs(colorFormats.size());
			
			for (uint32_t i = 0; i < colorFormats.size(); ++i) {
				VkAttachmentDescription &colorAttachment = attachmentDescs[i];
				uint8_t channels;
				colorAttachment.format = TranslateColorFormatToVulkan(colorFormats[i], channels);
				colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				colorAttachment.flags = 0;

				attachmentRefs[i].attachment = i;
				attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
			VkAttachmentReference *depthAttachmentRefPtr = nullptr;
			VkAttachmentReference depthAttachmentRef = {};
			if (depthFormat != DepthFormat::None) {
				VkAttachmentDescription &depthAttachment = attachmentDescs[colorFormats.size()];
				bool hasStencil = true;
				depthAttachment.format = TranslateDepthFormatToVulkan(depthFormat, hasStencil);
				hasStencil = true;
				depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
				depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				depthAttachment.finalLayout = hasStencil
					? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
					: VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
				depthAttachment.flags = 0;

				depthAttachmentRef.attachment = static_cast<uint32_t>(colorFormats.size());
				depthAttachmentRef.layout = hasStencil
					? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
					: VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
				depthAttachmentRefPtr = &depthAttachmentRef;
			}

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = static_cast<uint32_t>(attachmentRefs.size());
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
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
			renderPassInfo.pAttachments = attachmentDescs.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			if (vkCreateRenderPass(VulkanCore::Get().GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
				throw std::runtime_error("failed to create render pass!");
			}

			if (!debugName.empty()) {
				VulkanCore::Get().NameObject(VK_OBJECT_TYPE_RENDER_PASS, renderPass, debugName.c_str());
			}
		}

		VulkanRenderPass::~VulkanRenderPass() {
			Cleanup();
		}

		void VulkanRenderPass::Cleanup() {
			if (renderPass != nullptr) {
				vkDestroyRenderPass(VulkanCore::Get().GetDevice(), renderPass, nullptr);
				renderPass = nullptr;
			}
		}

		void VulkanRenderPass::Resize(uint32_t width, uint32_t height) {
			this->width = width;
			this->height = height;

			Cleanup();
			Create();
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
