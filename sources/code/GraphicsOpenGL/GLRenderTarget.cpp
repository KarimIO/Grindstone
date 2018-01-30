#include "GLRenderTarget.hpp"
#include <GL/gl3w.h>
#include "GLTexture.hpp"

GLRenderTarget::GLRenderTarget(RenderTargetCreateInfo *create_info, uint32_t count) {
    size_ = count;
	width_ = create_info[0].width;
	height_ = create_info[0].height;

    handles_ = new uint32_t[size_];
	glGenTextures(size_, handles_);
	format_ = new GLenum[size_];
    
    for (int i = 0; i < count; i++) {
        glBindTexture(GL_TEXTURE_2D, handles_[i]);

        GLint internalFormat;
        TranslateColorFormats(create_info[i].format, format_[i], internalFormat);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, create_info[i].width, create_info[i].height, 0, format_[i], GL_FLOAT, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

uint32_t GLRenderTarget::getHandle() {
    return handles_[0];
}

uint32_t GLRenderTarget::getHandle(uint32_t i) {
    return handles_[i];
}

uint32_t GLRenderTarget::getNumRenderTargets() {
    return size_;
}

void GLRenderTarget::Bind() {
    for (uint32_t i = 0; i < size_; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, handles_[i]);
    }
}

void GLRenderTarget::Bind(uint32_t j) {
	for (uint32_t i = 0; i < size_; i++) {
		glActiveTexture(GL_TEXTURE0 + j + i);
		glBindTexture(GL_TEXTURE_2D, handles_[i]);
	}
}

unsigned char *GLRenderTarget::RenderScreen(unsigned int i) {
	unsigned char *pixels = new unsigned char[width_ * height_ * 3];
	glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, width_, height_, format_[i], GL_UNSIGNED_BYTE,  pixels);
	return pixels;
}

GLRenderTarget::~GLRenderTarget() {
    glDeleteTextures(size_, handles_);
}