#include <iostream>
#include <cmath>
#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include <EngineCore/Logger.hpp>

#include "GLRenderTarget.hpp"
#include "GLFramebuffer.hpp"
#include "GLDepthStencilTarget.hpp"
#include "GLTexture.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::Framebuffer::Framebuffer(const Framebuffer::CreateInfo& createInfo) {
	if (createInfo.debugName != nullptr) {
		debugName = createInfo.debugName;
	}

	renderPass = createInfo.renderPass;
	colorAttachments.resize(createInfo.numRenderTargetLists);
	depthTarget = static_cast<DepthStencilTarget*>(createInfo.depthTarget);

	numTotalRenderTargets = createInfo.numRenderTargetLists;
	for (uint32_t i = 0; i < createInfo.numRenderTargetLists; i++) {
		colorAttachments[i] = createInfo.renderTargetLists[i];
	}

	CreateFramebuffer();
}

void OpenGL::Framebuffer::CreateFramebuffer() {
	if (framebuffer) {
		glDeleteFramebuffers(1, &framebuffer);
		framebuffer = 0;
	}

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	if (!debugName.empty()) {
		glObjectLabel(GL_FRAMEBUFFER, framebuffer, -1, debugName.c_str());
	}

	std::vector<GLenum> drawBuffers(numTotalRenderTargets);

	for (uint32_t i = 0; i < static_cast<uint32_t>(colorAttachments.size()); i++) {
		OpenGL::RenderTarget* renderTargetList = static_cast<OpenGL::RenderTarget*>(colorAttachments[i]);
		drawBuffers[i] = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i);
		if (renderTargetList->IsCubemap()) {
			for (int f = 0; f < 6; ++f) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffers[i], GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, renderTargetList->GetHandle(), 0);
			}
		}
		else {
			glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffers[i], GL_TEXTURE_2D, renderTargetList->GetHandle(), 0);
		}
	}

	if (depthTarget) {
		OpenGL::DepthStencilTarget* dt = static_cast<OpenGL::DepthStencilTarget*>(depthTarget);
		if (!dt->IsCubemap())
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dt->GetHandle(), 0);
		else
			for (int f = 0; f < 6; ++f) {
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, dt->GetHandle(), 0);
			}
	}

	if (numTotalRenderTargets > 0) {
		glDrawBuffers(numTotalRenderTargets, drawBuffers.data());
	}
	else {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		GPRINT_FATAL_V(LogSource::GraphicsAPI, "Framebuffer incomplete: {}.", debugName);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderPass* OpenGL::Framebuffer::GetRenderPass() const {
	return renderPass;
}

OpenGL::Framebuffer::~Framebuffer() {
	if (framebuffer != 0) {
		glDeleteFramebuffers(1, &framebuffer);
	}
}

uint32_t OpenGL::Framebuffer::GetAttachment(uint32_t attachmentIndex) {
	OpenGL::RenderTarget* rt = static_cast<OpenGL::RenderTarget*>(colorAttachments[attachmentIndex]);
	return rt->GetHandle();
}

void OpenGL::Framebuffer::Resize(uint32_t newWidth, uint32_t newHeight) {
	width = std::max(newWidth, 1u);
	height = std::max(newHeight, 1u);

	for (uint32_t i = 0; i < colorAttachments.size(); ++i) {
		OpenGL::RenderTarget* renderTargetList = static_cast<OpenGL::RenderTarget*>(colorAttachments[i]);
		renderTargetList->Resize(width, height);
	}

	if (depthTarget) {
		depthTarget->Resize(width, height);
	}

	CreateFramebuffer();
}

void OpenGL::Framebuffer::Clear(ClearMode mask) {
	int m = (((uint8_t)mask & (uint8_t)ClearMode::Depth) != 0) ? GL_DEPTH_BUFFER_BIT : 0;
	m = m | ((((uint8_t)mask & (uint8_t)ClearMode::Color) != 0) ? GL_COLOR_BUFFER_BIT : 0);
	glClear(m);
}

void OpenGL::Framebuffer::Blit(uint32_t i, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
	glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
	glBlitFramebuffer(x, y, x + w, y + h, x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void OpenGL::Framebuffer::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void OpenGL::Framebuffer::BindWrite() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
}

void OpenGL::Framebuffer::BindRead() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
}

void OpenGL::Framebuffer::BindTextures(int k) {
	for (size_t i = 0; i < colorAttachments.size(); i++) {
		OpenGL::RenderTarget* rt = static_cast<OpenGL::RenderTarget*>(colorAttachments[i]);
		rt->Bind(i);
	}

	if (depthTarget) {
		OpenGL::DepthStencilTarget* dt = static_cast<OpenGL::DepthStencilTarget*>(depthTarget);
		dt->Bind(colorAttachments.size());
	}
}

void OpenGL::Framebuffer::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32_t OpenGL::Framebuffer::GetWidth() const {
	return width;
}

uint32_t OpenGL::Framebuffer::GetHeight() const {
	return height;
}

uint32_t OpenGL::Framebuffer::GetRenderTargetCount() const {
	return static_cast<uint32_t>(colorAttachments.size());
}

Grindstone::GraphicsAPI::RenderTarget* OpenGL::Framebuffer::GetRenderTarget(uint32_t index) const {
	return colorAttachments[index];
}

Grindstone::GraphicsAPI::DepthStencilTarget* OpenGL::Framebuffer::GetDepthStencilTarget() const {
	return depthTarget;
}
