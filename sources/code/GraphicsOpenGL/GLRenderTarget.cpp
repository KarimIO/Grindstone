#include "GLRenderTarget.hpp"
#include <GL/gl3w.h>
#include "GLTexture.hpp"

GLRenderTarget::GLRenderTarget(RenderTargetCreateInfo *create_info, uint32_t count) {
    size_ = count;

    handles_ = new uint32_t[size_];
	glGenTextures(size_, handles_);
    
    for (int i = 0; i < count; i++) {
        glBindTexture(GL_TEXTURE_2D, handles_[i]);

        GLint internalFormat;
        GLenum format;
        TranslateFormats(create_info[i].format, format, internalFormat);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, create_info[i].width, create_info[i].height, 0, format, GL_FLOAT, 0);

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

void GLRenderTarget::Bind(uint32_t i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, handles_[i]);
}

GLRenderTarget::~GLRenderTarget() {
    glDeleteTextures(size_, handles_);
}