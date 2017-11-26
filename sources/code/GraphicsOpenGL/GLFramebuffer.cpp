#include <GL/gl3w.h>
#include <iostream>
#include "GLFramebuffer.hpp"
#include "GLTexture.hpp"
#include "GLRenderTarget.hpp"

GLFramebuffer::GLFramebuffer(FramebufferCreateInfo create_info) {
	glGenFramebuffers(1, &fbo_);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

	uint32_t num_targets = 0;
	for (uint32_t i = 0; i < create_info.num_render_target_lists; i++) {
		num_targets += static_cast<GLRenderTarget *>(create_info.render_target_lists[i])->getNumRenderTargets();
	}

	GLenum *DrawBuffers = new GLenum[num_targets];
	uint32_t k = 0;
	for (uint32_t i = 0; i < create_info.num_render_target_lists; i++) {
		GLRenderTarget *render_target_list = static_cast<GLRenderTarget *>(create_info.render_target_lists[i]);
		for (size_t j = 0; j < render_target_list->getNumRenderTargets(); j++) {
			DrawBuffers[k] = GL_COLOR_ATTACHMENT0 + k;
			glFramebufferTexture2D(GL_FRAMEBUFFER, DrawBuffers[k++], GL_TEXTURE_2D, render_target_list->getHandle(j), 0);
		}
	}

	if (create_info.depth_target) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, static_cast<GLRenderTarget *>(create_info.depth_target)->getHandle(), 0);
	}

	if (num_targets > 0)
		glDrawBuffers(num_targets, DrawBuffers);
	else
		glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer Incomplete\n");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	delete[] DrawBuffers;
}

GLFramebuffer::~GLFramebuffer() {
	if (fbo_ != 0) {
		glDeleteFramebuffers(1, &fbo_);
	}
}

void GLFramebuffer::Clear() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void GLFramebuffer::CopyFrom(Framebuffer *) {

}

void GLFramebuffer::Bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
}

void GLFramebuffer::BindWrite() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
}

void GLFramebuffer::BindRead() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
}

void GLFramebuffer::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
