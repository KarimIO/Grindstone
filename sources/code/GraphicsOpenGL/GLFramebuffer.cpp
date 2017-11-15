#include <GL/gl3w.h>
#include <iostream>
#include "GLFramebuffer.hpp"
#include "GLTexture.hpp"

GLFramebuffer::GLFramebuffer(FramebufferCreateInfo createInfo) {
	m_width = createInfo.width;
	m_height = createInfo.height;

	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	m_numTextures = createInfo.numColorTargets;

	m_textures = new GLuint[createInfo.numColorTargets];
	glGenTextures(createInfo.numColorTargets, m_textures);
	GLenum *DrawBuffers = new GLenum[createInfo.numColorTargets];
	for (uint32_t i = 0; i < createInfo.numColorTargets; i++) {
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);

		GLint internalFormat;
		GLenum format;
		TranslateFormats(createInfo.colorFormats[i], format, internalFormat);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, createInfo.width, createInfo.height, 0, format, GL_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_textures[i], 0);
	}

	if (createInfo.depthFormat != FORMAT_DEPTH_NONE) {
		GLenum depthFormat;
		switch (createInfo.depthFormat) {
		case FORMAT_DEPTH_16:
			depthFormat = GL_DEPTH_COMPONENT16;
			break;
		case FORMAT_DEPTH_24:
			depthFormat = GL_DEPTH_COMPONENT24;
			break;
		case FORMAT_DEPTH_32:
			depthFormat = GL_DEPTH_COMPONENT32F;
			break;
			/*case FORMAT_DEPTH_16_STENCIL_8:
				depthFormat = GL_DEPTH24_STENCIL8;
				break;*/
		case FORMAT_DEPTH_24_STENCIL_8:
			depthFormat = GL_DEPTH24_STENCIL8;
			break;
		case FORMAT_DEPTH_32_STENCIL_8:
			depthFormat = GL_DEPTH32F_STENCIL8;
			break;
			/*case FORMAT_STENCIL_8:
				depthFormat = GL_DEPTH32F_STENCIL8;
				break;*/
		}

		glGenTextures(1, &m_depthTexture);
		glBindTexture(GL_TEXTURE_2D, m_depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, depthFormat, createInfo.width, createInfo.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
	}
	else
		m_depthTexture = 0;

	glBindTexture(GL_TEXTURE_2D, 0);

	if (createInfo.numColorTargets > 0)
		glDrawBuffers(createInfo.numColorTargets, DrawBuffers);
	else
		glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer Incomplete\n";

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	delete[] DrawBuffers;
}

GLFramebuffer::~GLFramebuffer() {
	if (m_fbo != 0) {
		glDeleteFramebuffers(1, &m_fbo);
	}

	if (m_numTextures > 0 && m_textures[0] != 0) {
		glDeleteTextures(m_numTextures, m_textures);
	}

	if (m_depthTexture != 0) {
		glDeleteTextures(1, &m_depthTexture);
	}
}

void GLFramebuffer::Clear() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void GLFramebuffer::Blit(int i, int x, int y, int w, int h) {
	if (i == -1) {
		glReadBuffer(GL_NONE);
		glBlitFramebuffer(0, 0, m_width, m_height, x, y, x + w, y + h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}
	else {
		glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
		glBlitFramebuffer(0, 0, m_width, m_height, x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
}

void GLFramebuffer::BindWrite() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void GLFramebuffer::BindRead() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
}

void GLFramebuffer::BindTextures() {
	for (int i = 0; i < m_numTextures; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
	}

	glActiveTexture(GL_TEXTURE0 + m_numTextures);
	glBindTexture(GL_TEXTURE_2D, m_depthTexture);
}

void GLFramebuffer::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
