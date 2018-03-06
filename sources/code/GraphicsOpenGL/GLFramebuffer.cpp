#include <GL/gl3w.h>
#include <iostream>
#include "GLFramebuffer.hpp"
#include "GLTexture.hpp"
#include <cmath>
#include <../deps/glm/glm.hpp>

GLFramebuffer::GLFramebuffer(FramebufferCreateInfo create_info) {
	render_target_lists_ = (GLRenderTarget **)(create_info.render_target_lists); 
	num_render_target_lists_ = create_info.num_render_target_lists;
	depth_target_ = (GLDepthTarget *)create_info.depth_target;

	glGenFramebuffers(1, &fbo_);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

	num_total_render_targets = 0;
	for (uint32_t i = 0; i < create_info.num_render_target_lists; i++) {
		num_total_render_targets += static_cast<GLRenderTarget *>(create_info.render_target_lists[i])->getNumRenderTargets();
	}

	GLenum *DrawBuffers = new GLenum[num_total_render_targets];
	uint32_t k = 0;
	for (uint32_t i = 0; i < create_info.num_render_target_lists; i++) {
		GLRenderTarget *render_target_list = static_cast<GLRenderTarget *>(create_info.render_target_lists[i]);
		for (size_t j = 0; j < render_target_list->getNumRenderTargets(); j++) {
			DrawBuffers[k] = GL_COLOR_ATTACHMENT0 + k;
			glFramebufferTexture2D(GL_FRAMEBUFFER, DrawBuffers[k++], GL_TEXTURE_2D, render_target_list->getHandle(j), 0);
		}
	}

	if (create_info.depth_target) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, static_cast<GLDepthTarget *>(create_info.depth_target)->getHandle(), 0);
	}

	if (num_total_render_targets > 0)
		glDrawBuffers(num_total_render_targets, DrawBuffers);
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

float GLFramebuffer::getExposure(int i) {
	int width_ = 640;
	int height_ = 480;
    unsigned int s = width_ * height_ * 3;

    GLfloat *values = new GLfloat[s];
	glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width_, height_, GL_RGB, GL_FLOAT, values);

    float val = 0;
    for (int i = 0; i < s; i+=3) {
		float lum = (values[i] + values[i + 1] + values[i + 2]);//glm::dot(glm::vec3(values[i], values[i+1], values[i+2]), glm::vec3(0.3, 0.59, 0.11));
		val += lum; // std::log2(lum);
    }

	val /= float(s);

    delete[] values;

    return val;
}

void GLFramebuffer::Blit(uint32_t i, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
	glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
	glBlitFramebuffer(x, y, x + w, y + h, x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GLFramebuffer::CopyFrom(Framebuffer *) {

}

void GLFramebuffer::Bind(bool depth) {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	glDepthMask(depth ? GL_TRUE : GL_FALSE);
}

void GLFramebuffer::BindWrite(bool depth) {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);
	glDepthMask(depth ? GL_TRUE : GL_FALSE);
}

void GLFramebuffer::BindRead() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
}

void GLFramebuffer::BindTextures(int k) {
	int j = k;
	for (int i = 0; i < num_render_target_lists_; i++) {
		render_target_lists_[i]->Bind(j);
		j += render_target_lists_[i]->getNumRenderTargets();
	}

	if (depth_target_)
		depth_target_->Bind(j);
}

void GLFramebuffer::Unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
