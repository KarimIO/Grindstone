#include <assert.h>

#include <EngineCore/Logger.hpp>

#include "VulkanRenderPass.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanDepthStencilTarget.hpp"
#include "VulkanCore.hpp"
#include "VulkanFramebuffer.hpp"

namespace Vulkan = Grindstone::GraphicsAPI::Vulkan;

Vulkan::Framebuffer::Framebuffer(
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
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_FRAMEBUFFER, framebuffer, debugName);
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Framebuffer!");
	}
}

Vulkan::Framebuffer::Framebuffer(const CreateInfo& createInfo) :
	isCubemap(createInfo.isCubemap),
	renderPass(createInfo.renderPass),
	width(createInfo.width),
	height(createInfo.height)
{
	if (createInfo.debugName != nullptr) {
		debugName = createInfo.debugName;
	}

	std::vector<VkImageView> attachments;
	for (uint32_t i = 0; i < createInfo.numRenderTargetLists; ++i) {
		colorAttachments.push_back(static_cast<Vulkan::RenderTarget*>(createInfo.renderTargetLists[i]));
	}

	if (createInfo.depthTarget != nullptr) {
		depthAttachment = static_cast<Vulkan::DepthStencilTarget*>(createInfo.depthTarget);
	}

	Create();
}

Vulkan::Framebuffer::~Framebuffer() {
	Cleanup();
}

void Vulkan::Framebuffer::UpdateNativeFramebuffer(
	RenderPass* renderPass,
	VkFramebuffer framebuffer,
	uint32_t width,
	uint32_t height
) {
	this->renderPass = renderPass;
	this->framebuffer = framebuffer;
	this->width = width;
	this->height = height;

	Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_FRAMEBUFFER, framebuffer, debugName.c_str());
}

void Vulkan::Framebuffer::Cleanup() {
	if (framebuffer != nullptr) {
		vkDestroyFramebuffer(Vulkan::Core::Get().GetDevice(), framebuffer, nullptr);
		framebuffer = nullptr;
	}
}

VkFramebuffer Vulkan::Framebuffer::GetFramebuffer() const {
	return framebuffer;
}

Grindstone::GraphicsAPI::RenderPass* Vulkan::Framebuffer::GetRenderPass() const {
	return renderPass;
}

uint32_t Vulkan::Framebuffer::GetAttachment(uint32_t attachmentIndex) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Framebuffer::GetAttachment is not used.");
	assert(false);
	return 0;
}

void Vulkan::Framebuffer::Resize(uint32_t width, uint32_t height) {
	Cleanup();

	this->width = width;
	this->height = height;
	Create();
}

void Vulkan::Framebuffer::Create() {
	std::vector<VkImageView> attachments;
	for (uint32_t i = 0; i < colorAttachments.size(); ++i) {
		attachments.emplace_back(colorAttachments[i]->GetImageView());
	}

	if (depthAttachment != nullptr) {
		attachments.emplace_back(depthAttachment->GetImageView());
	}

	Vulkan::RenderPass* rp = static_cast<Vulkan::RenderPass*>(renderPass);

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = rp->GetRenderPassHandle();
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.width = width;
	framebufferInfo.height = height;
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(Vulkan::Core::Get().GetDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
		GPRINT_FATAL(LogSource::GraphicsAPI, "failed to create framebuffer!");
	}

	if (!debugName.empty()) {
		Vulkan::Core::Get().NameObject(VK_OBJECT_TYPE_FRAMEBUFFER, framebuffer, debugName.c_str());
	}
	else {
		GPRINT_FATAL(LogSource::GraphicsAPI, "Unnamed Framebuffer!");
	}
}

uint32_t Vulkan::Framebuffer::GetWidth() const {
	return width;
}

uint32_t Vulkan::Framebuffer::GetHeight() const {
	return height;
}

uint32_t Vulkan::Framebuffer::GetRenderTargetCount() const {
	return static_cast<uint32_t>(colorAttachments.size());
}

Grindstone::GraphicsAPI::RenderTarget* Vulkan::Framebuffer::GetRenderTarget(uint32_t index) const {
	return colorAttachments[index];
}

Grindstone::GraphicsAPI::DepthStencilTarget* Vulkan::Framebuffer::GetDepthStencilTarget() const {
	return depthAttachment;
}

void Vulkan::Framebuffer::Clear(GraphicsAPI::ClearMode mask) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Framebuffer::Clear is not used.");
	assert(false);
}

void Vulkan::Framebuffer::Bind() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Framebuffer::Bind is not used.");
	assert(false);
}

void Vulkan::Framebuffer::BindWrite() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Framebuffer::BindWrite is not used.");
	assert(false);
}

void Vulkan::Framebuffer::BindRead() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Framebuffer::BindRead is not used.");
	assert(false);
}

void Vulkan::Framebuffer::BindTextures(int i) {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Framebuffer::BindTextures is not used.");
	assert(false);
}

void Vulkan::Framebuffer::Unbind() {
	GPRINT_FATAL(LogSource::GraphicsAPI, "Framebuffer::Unbind is not used.");
	assert(false);
}
