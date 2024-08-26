#include <assert.h>

#include <EngineCore/Logger.hpp>

#include "VulkanRenderPass.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanDepthTarget.hpp"
#include "VulkanCore.hpp"
#include "VulkanFramebuffer.hpp"

using namespace Grindstone::GraphicsAPI;

VulkanFramebuffer::VulkanFramebuffer(
	RenderPass* renderPass,
	VkFramebuffer framebuffer,
	uint32_t width,
	uint32_t height,
	const char* debugName
) : renderPass(renderPass),
	framebuffer(framebuffer),
	width(width),
	height(height) {
	if (debugName != nullptr) {
		this->debugName = debugName;
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_FRAMEBUFFER, framebuffer, debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Framebuffer!");
	}
}

VulkanFramebuffer::VulkanFramebuffer(Framebuffer::CreateInfo& createInfo) : isCubemap(createInfo.isCubemap), renderPass(createInfo.renderPass), width(createInfo.width), height(createInfo.height) {
	if (createInfo.debugName != nullptr) {
		debugName = createInfo.debugName;
	}

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

void VulkanFramebuffer::UpdateNativeFramebuffer(
	RenderPass* renderPass,
	VkFramebuffer framebuffer,
	uint32_t width,
	uint32_t height
) {
	this->renderPass = renderPass;
	this->framebuffer = framebuffer;
	this->width = width;
	this->height = height;

	VulkanCore::Get().NameObject(VK_OBJECT_TYPE_FRAMEBUFFER, framebuffer, debugName.c_str());
}

void VulkanFramebuffer::Cleanup() {
	if (framebuffer != nullptr) {
		vkDestroyFramebuffer(VulkanCore::Get().GetDevice(), framebuffer, nullptr);
		framebuffer = nullptr;
	}
}

VkFramebuffer VulkanFramebuffer::GetFramebuffer() const {
	return framebuffer;
}

RenderPass* VulkanFramebuffer::GetRenderPass() const {
	return renderPass;
}

uint32_t VulkanFramebuffer::GetAttachment(uint32_t attachmentIndex) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanFramebuffer::GetAttachment is not used.");
	assert(false);
	return 0;
}

void VulkanFramebuffer::Resize(uint32_t width, uint32_t height) {
	Cleanup();

	this->width = width;
	this->height = height;
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
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(VulkanCore::Get().GetDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create framebuffer!");
	}

	if (!debugName.empty()) {
		VulkanCore::Get().NameObject(VK_OBJECT_TYPE_FRAMEBUFFER, framebuffer, debugName.c_str());
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Framebuffer!");
	}
}

uint32_t VulkanFramebuffer::GetWidth() const {
	return width;
}

uint32_t VulkanFramebuffer::GetHeight() const {
	return height;
}

uint32_t VulkanFramebuffer::GetRenderTargetCount() const {
	return static_cast<uint32_t>(colorAttachments.size());
}

RenderTarget* VulkanFramebuffer::GetRenderTarget(uint32_t index) const {
	return colorAttachments[index];
}

DepthTarget* VulkanFramebuffer::GetDepthTarget() const {
	return depthAttachment;
}

void VulkanFramebuffer::Clear(GraphicsAPI::ClearMode mask) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanFramebuffer::Clear is not used.");
	assert(false);
}

void VulkanFramebuffer::Bind() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanFramebuffer::Bind is not used.");
	assert(false);
}

void VulkanFramebuffer::BindWrite() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanFramebuffer::BindWrite is not used.");
	assert(false);
}

void VulkanFramebuffer::BindRead() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanFramebuffer::BindRead is not used.");
	assert(false);
}

void VulkanFramebuffer::BindTextures(int i) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanFramebuffer::BindTextures is not used.");
	assert(false);
}

void VulkanFramebuffer::Unbind() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "VulkanFramebuffer::Unbind is not used.");
	assert(false);
}
