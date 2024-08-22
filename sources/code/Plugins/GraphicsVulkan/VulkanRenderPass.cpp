#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include "VulkanCore.hpp"
#include "VulkanFormat.hpp"
#include "VulkanRenderPass.hpp"

namespace Grindstone::GraphicsAPI {
	VulkanRenderPass::VulkanRenderPass(VkRenderPass renderPass, const char* debugName)
		: renderPass(renderPass) {

		if (debugName != nullptr) {
			this->debugName = debugName;

			VulkanCore::Get().NameObject(VK_OBJECT_TYPE_RENDER_PASS, renderPass, debugName);
		}
		else {
			GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed RenderPass!");
		}
	}

	VulkanRenderPass::VulkanRenderPass(RenderPass::CreateInfo& createInfo) : shouldClearDepthOnLoad(createInfo.shouldClearDepthOnLoad) {
		if (createInfo.debugName != nullptr) {
			debugName = createInfo.debugName;
		}

		if (createInfo.debugColor != nullptr) {
			memcpy(debugColor, createInfo.debugColor, sizeof(debugColor));
		}

		for (uint32_t i = 0; i < createInfo.colorAttachmentCount; ++i) {
			colorAttachments.push_back(createInfo.colorAttachments[i]);
		}

		depthFormat = createInfo.depthFormat;

		Create();
	}

	void VulkanRenderPass::Update(VkRenderPass renderPass) {
		this->renderPass = renderPass;
	}

	void VulkanRenderPass::Create() {
		uint32_t total = static_cast<uint32_t>(colorAttachments.size());
		total += (depthFormat != DepthFormat::None) ? 1 : 0;

		std::vector<VkAttachmentDescription> attachmentDescs(total);
		std::vector<VkAttachmentReference> attachmentRefs(colorAttachments.size());
			
		for (uint32_t i = 0; i < colorAttachments.size(); ++i) {
			VkAttachmentDescription &colorAttachment = attachmentDescs[i];
			uint8_t channels;
			colorAttachment.format = TranslateColorFormatToVulkan(colorAttachments[i].colorFormat, channels);
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = colorAttachments[i].shouldClear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = colorAttachments[i].shouldClear ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			colorAttachment.flags = 0;

			attachmentRefs[i].attachment = i;
			attachmentRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		VkAttachmentReference *depthAttachmentRefPtr = nullptr;
		VkAttachmentReference depthAttachmentRef = {};
		if (depthFormat != DepthFormat::None) {
			VkAttachmentDescription &depthAttachment = attachmentDescs[colorAttachments.size()];
			bool hasStencil = true;
			depthAttachment.format = TranslateDepthFormatToVulkan(depthFormat, hasStencil);
			hasStencil = true;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = shouldClearDepthOnLoad
				? VK_ATTACHMENT_LOAD_OP_CLEAR
				: VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.finalLayout = hasStencil
				? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
				: VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
			depthAttachment.initialLayout = shouldClearDepthOnLoad
				? VK_IMAGE_LAYOUT_UNDEFINED
				: depthAttachment.finalLayout;
			depthAttachment.flags = 0;

			depthAttachmentRef.attachment = static_cast<uint32_t>(colorAttachments.size()); // Attaches after every color attachment
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
			GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create render pass!");
		}

		if (!debugName.empty()) {
			VulkanCore::Get().NameObject(VK_OBJECT_TYPE_RENDER_PASS, renderPass, debugName.c_str());
		}
		else {
			GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed RenderPass!");
		}
	}

	const char* VulkanRenderPass::GetDebugName() const {
		return debugName.c_str();
	}

	const float* VulkanRenderPass::GetDebugColor() const {
		return &debugColor[0];
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

	VkRenderPass VulkanRenderPass::GetRenderPassHandle() const {
		return renderPass;
	}
}
