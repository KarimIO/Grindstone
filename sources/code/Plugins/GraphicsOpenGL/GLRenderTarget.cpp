#include <iostream>
#include <cmath>
#include <GL/gl3w.h>
#include <glm/glm.hpp>

#include "GLRenderTarget.hpp"
#include "GLFormats.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::RenderTarget::RenderTarget(const RenderTarget::CreateInfo& createInfo) {
	width = std::max(createInfo.width, 1u);
	height = std::max(createInfo.height, 1u);
	this->isCubemap = false;

	OpenGLFormats oglFormat = TranslateFormatToOpenGL(createInfo.format);
	type = oglFormat.type;
	format = oglFormat.format;
	internalFormat = oglFormat.internalFormat;

	glDeleteTextures(1, &renderTargetHandle);
	glGenTextures(1, &renderTargetHandle);

	if (isCubemap) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, renderTargetHandle);

		for (unsigned int f = 0; f < 6u; ++f) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, internalFormat, width, height, 0, format, type, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR_MIPMAP_LINEAR
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, renderTargetHandle);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}

uint32_t OpenGL::RenderTarget::GetHandle() const {
	return renderTargetHandle;
}

bool OpenGL::RenderTarget::IsCubemap() const {
	return isCubemap;
}

void OpenGL::RenderTarget::Resize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;
}

void OpenGL::RenderTarget::Bind() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, renderTargetHandle);
}

void OpenGL::RenderTarget::Bind(uint32_t index) {
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, renderTargetHandle);
}

void OpenGL::RenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) {
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, resx, resy, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

OpenGL::RenderTarget::~RenderTarget() {
	if (renderTargetHandle != 0) {
		glDeleteTextures(1, &renderTargetHandle);
	}
}
