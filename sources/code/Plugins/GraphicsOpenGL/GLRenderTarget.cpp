#include <iostream>
#include <cmath>
#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include "GLRenderTarget.hpp"
#include "GLFormats.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::RenderTarget::RenderTarget(const RenderTarget::CreateInfo& createInfoList) {
	renderTargetCount = 1;
	width = std::max(createInfoList.width, 1u);
	height = std::max(createInfoList.height, 1u);
	this->isCubemap = false;

	renderTargetHandles.resize(renderTargetCount);
	formats.resize(renderTargetCount);
	internalFormats.resize(renderTargetCount);

	for (uint32_t i = 0; i < renderTargetCount; i++) {
		bool isCompressed;
		TranslateColorFormatToOpenGL(createInfoList.format, isCompressed, formats[i], internalFormats[i]);
	}

	CreateRenderTargets();
}

OpenGL::RenderTarget::RenderTarget(const RenderTarget::CreateInfo* createInfoList, uint32_t createInfoCount, bool isCubemap) {
	renderTargetCount = createInfoCount;
	width = std::max(createInfoList[0].width, 1u);
	height = std::max(createInfoList[0].height, 1u);
	this->isCubemap = isCubemap;

	renderTargetHandles.resize(renderTargetCount);
	formats.resize(renderTargetCount);
	internalFormats.resize(renderTargetCount);

	for (uint32_t i = 0; i < renderTargetCount; i++) {
		bool isCompressed;
		TranslateColorFormatToOpenGL(createInfoList[i].format, isCompressed, formats[i], internalFormats[i]);
	}

	CreateRenderTargets();
}

void OpenGL::RenderTarget::CreateRenderTargets() {
	if (renderTargetHandles[0] > 0) {
		glDeleteTextures(renderTargetCount, renderTargetHandles.data());

		for (uint32_t i = 0; i < renderTargetCount; i++) {
			renderTargetHandles[i] = 0;
		}
	}

	glGenTextures(renderTargetCount, renderTargetHandles.data());
	if (isCubemap) {
		for (uint32_t i = 0; i < renderTargetCount; i++) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, renderTargetHandles[i]);

			for (unsigned int f = 0; f < 6u; ++f) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, internalFormats[i], width, height, 0, formats[i], GL_FLOAT, nullptr);
			}

			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR_MIPMAP_LINEAR
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}
	else {
		for (uint32_t i = 0; i < renderTargetCount; i++) {
			glBindTexture(GL_TEXTURE_2D, renderTargetHandles[i]);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormats[i], width, height, 0, formats[i], GL_FLOAT, 0);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
	}
}

uint32_t OpenGL::RenderTarget::GetHandle() const {
	return renderTargetHandles[0];
}

uint32_t OpenGL::RenderTarget::GetHandle(uint32_t i) const {
	return renderTargetHandles[i];
}

uint32_t OpenGL::RenderTarget::GetNumRenderTargets() const {
	return renderTargetCount;
}

bool OpenGL::RenderTarget::IsCubemap() const {
	return isCubemap;
}

void OpenGL::RenderTarget::Resize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;

	CreateRenderTargets();
}

void OpenGL::RenderTarget::Bind() {
	for (uint32_t i = 0; i < renderTargetCount; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, renderTargetHandles[i]);
	}
}

void OpenGL::RenderTarget::Bind(uint32_t j) {
	for (uint32_t i = 0; i < renderTargetCount; i++) {
		glActiveTexture(GL_TEXTURE0 + j + i);
		glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, renderTargetHandles[i]);
	}
}

void OpenGL::RenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) {
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, resx, resy, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

OpenGL::RenderTarget::~RenderTarget() {
	glDeleteTextures(renderTargetCount, renderTargetHandles.data());
}
