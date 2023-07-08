#include "VulkanRenderPass.hpp"
#include "VulkanFramebuffer.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanDepthTarget.hpp"
#include "VulkanCore.hpp"

#include <assert.h>

using namespace Grindstone::GraphicsAPI;

VulkanFramebuffer::VulkanFramebuffer(RenderPass* renderPass, VkFramebuffer framebuffer) {
	this->renderPass = renderPass;
	this->framebuffer = framebuffer;
}

VulkanFramebuffer::VulkanFramebuffer(Framebuffer::CreateInfo& createInfo) {
	if (createInfo.debugName != nullptr) {
		debugName = createInfo.debugName;
	}

	renderPass = createInfo.renderPass;

	std::vector<VkImageView> attachments;
	for (uint32_t i = 0; i < createInfo.numRenderTargetLists; ++i) {
		colorAttachments.push_back(static_cast<VulkanRenderTarget*>(createInfo.renderTargetLists[i]));
	}

	if (createInfo.depthTarget != nullptr) {
		depthAttachment = static_cast<VulkanDepthTarget*>(createInfo.depthTarget);
	}

	Create();
}

VulkanFramebuffer::~VulkanFramebuffer() {
	Cleanup();
}

void VulkanFramebuffer::Cleanup() {
	if (framebuffer != nullptr) {
		vkDestroyFramebuffer(VulkanCore::Get().GetDevice(), framebuffer, nullptr);
		framebuffer = nullptr;
	}
}

VkFramebuffer VulkanFramebuffer::GetFramebuffer() {
	return framebuffer;
}

RenderPass* VulkanFramebuffer::GetRenderPass() {
	return renderPass;
}

uint32_t VulkanFramebuffer::GetAttachment(uint32_t attachmentIndex) {
	std::cout << "VulkanFramebuffer::GetAttachment is not used.\n";
	assert(false);
	return 0;
}

void VulkanFramebuffer::Resize(uint32_t width, uint32_t height) {
	Cleanup();
	Create();
}

void VulkanFramebuffer::Create() {
	std::vector<VkImageView> attachments;
	for (uint32_t i = 0; i < colorAttachments.size(); ++i) {
		attachments.emplace_back(colorAttachments[i]->GetImageView());
	}

	if (depthAttachment != nullptr) {
		attachments.emplace_back(depthAttachment->GetImageView());
	}

	VulkanRenderPass* rp = static_cast<VulkanRenderPass*>(renderPass);

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = rp->GetRenderPassHandle();
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.width = rp->GetWidth();
	framebufferInfo.height = rp->GetHeight();
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(VulkanCore::Get().GetDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer!");
	}

	if (!debugName.empty()) {

	}
}

void VulkanFramebuffer::Clear(GraphicsAPI::ClearMode mask) {
	std::cout << "VulkanFramebuffer::Clear is not used.\n";
	assert(false);
}
void VulkanFramebuffer::Bind() {
	std::cout << "VulkanFramebuffer::Bind is not used.\n";
	assert(false);
}
void VulkanFramebuffer::BindWrite() {
	std::cout << "VulkanFramebuffer::BindWrite is not used.\n";
	assert(false);
}
void VulkanFramebuffer::BindRead() {
	std::cout << "VulkanFramebuffer::BindRead is not used.\n";
	assert(false);
}
void VulkanFramebuffer::BindTextures(int i) {
	std::cout << "VulkanFramebuffer::BindTextures is not used.\n";
	assert(false);
}
void VulkanFramebuffer::Unbind() {
	std::cout << "VulkanFramebuffer::Unbind is not used.\n";
	assert(false);
}
