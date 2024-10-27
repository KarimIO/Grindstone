#include <GL/gl3w.h>

#include "GLDepthStencilTarget.hpp"
#include "GLFormats.hpp"

using namespace Grindstone::GraphicsAPI;

OpenGL::DepthStencilTarget::DepthStencilTarget(const DepthStencilTarget::CreateInfo& createInfo) {
	width = createInfo.width;
	height = createInfo.height;
	depthFormat = createInfo.format;
	isCubemap = createInfo.isCubemap;
	isShadowMap = createInfo.isShadowMap;

	CreateDepthStencilTarget();
}

void OpenGL::DepthStencilTarget::CreateDepthStencilTarget() {
	if (handle) {
		glDeleteTextures(1, &handle);
	}

	glGenTextures(1, &handle);
	if (isCubemap) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, handle);

		GLint internalFormat;
		GLenum format;
		TranslateDepthFormatToOpenGL(depthFormat, format, internalFormat);

		for (GLenum i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, width, height, 0, format, GL_FLOAT, 0);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (isShadowMap) {
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else {
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}
	else {
		glBindTexture(GL_TEXTURE_2D, handle);

		GLint internalFormat;
		GLenum format;
		TranslateDepthFormatToOpenGL(depthFormat, format, internalFormat);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		if (isShadowMap) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
	}
}

uint32_t OpenGL::DepthStencilTarget::GetHandle() const {
	return handle;
}

bool OpenGL::DepthStencilTarget::IsCubemap() const {
	return isCubemap;
}

void OpenGL::DepthStencilTarget::Bind(int i) {
	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, handle);
}

void OpenGL::DepthStencilTarget::Resize(uint32_t width, uint32_t height) {
	this->width = width;
	this->height = height;

	CreateDepthStencilTarget();
}

void OpenGL::DepthStencilTarget::BindFace(int k) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + k, handle, 0);
	glDrawBuffer(GL_NONE);
}

OpenGL::DepthStencilTarget::~DepthStencilTarget() {
	glDeleteTextures(1, &handle);
}
