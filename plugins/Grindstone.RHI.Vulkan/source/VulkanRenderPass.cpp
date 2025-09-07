#include <vulkan/vulkan.h>

#include <EngineCore/Logger.hpp>

#include <Grindstone.RHI.Vulkan/include/VulkanCore.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanFormat.hpp>
#include <Grindstone.RHI.Vulkan/include/VulkanRenderPass.hpp>

namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::RenderPass::RenderPass(VkRenderPass renderPass, const char* debugName)
	: renderPass(renderPass), ownsRenderPass(false) {

	if (debugName != nullptr) {
		this->debugName = debugName;

		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_RENDER_PASS, renderPass, debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed RenderPass!");
	}
}

Vulkan::RenderPass::RenderPass(const CreateInfo& createInfo)
	: shouldClearDepthOnLoad(createInfo.shouldClearDepthOnLoad), ownsRenderPass(true) {
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

void Vulkan::RenderPass::Update(VkRenderPass renderPass) {
	this->renderPass = renderPass;
}

void Vulkan::RenderPass::Create() {
	uint32_t total = static_cast<uint32_t>(colorAttachments.size());
	total += (depthFormat != Format::Invalid) ? 1 : 0;

	std::vector<VkAttachmentDescription> attachmentDescs(total);
	std::vector<VkAttachmentReference> attachmentRefs(colorAttachments.size());
			
	for (uint32_t i = 0; i < colorAttachments.size(); ++i) {
		VkAttachmentDescription &colorAttachment = attachmentDescs[i];
		colorAttachment.format = TranslateFormatToVulkan(colorAttachments[i].colorFormat);
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
	if (depthFormat != Format::Invalid) {
		FormatDepthStencilType depthStencilType = GetFormatDepthStencilType(depthFormat);
		VkAttachmentDescription &depthAttachment = attachmentDescs[colorAttachments.size()];
		depthAttachment.format = TranslateFormatToVulkan(depthFormat);
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = shouldClearDepthOnLoad
			? VK_ATTACHMENT_LOAD_OP_CLEAR
			: VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		depthAttachment.initialLayout = shouldClearDepthOnLoad
			? VK_IMAGE_LAYOUT_UNDEFINED
			: depthAttachment.finalLayout;
		depthAttachment.flags = 0;

		depthAttachmentRef.attachment = static_cast<uint32_t>(colorAttachments.size()); // Attaches after every color attachment
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
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

	if (vkCreateRenderPass(Vulkan::Core::Get().GetDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create render pass!");
	}

	if (!debugName.empty()) {
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_RENDER_PASS, renderPass, debugName.c_str());
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed RenderPass!");
	}
}

const char* Vulkan::RenderPass::GetDebugName() const {
	return debugName.c_str();
}

const float* Vulkan::RenderPass::GetDebugColor() const {
	return &debugColor[0];
}

Vulkan::RenderPass::~RenderPass() {
	Cleanup();
}

void Vulkan::RenderPass::Cleanup() {
	if (ownsRenderPass && renderPass != nullptr) {
		vkDestroyRenderPass(Vulkan::Core::Get().GetDevice(), renderPass, nullptr);
		renderPass = nullptr;
	}
}

VkRenderPass Vulkan::RenderPass::GetRenderPassHandle() const {
	return renderPass;
}
