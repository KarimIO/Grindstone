#include "GLDepthTarget.hpp"
#include <GL/gl3w.h>
#include "GLTexture.hpp"

GLDepthTarget::GLDepthTarget(DepthTargetCreateInfo *create_info, uint32_t count) {
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

uint32_t GLDepthTarget::getHandle() {
    return handles_[0];
}

uint32_t GLDepthTarget::getHandle(uint32_t i) {
    return handles_[i];
}

uint32_t GLDepthTarget::getNumDepthTargets() {
    return size_;
}

void GLDepthTarget::Bind() {
    for (uint32_t i = 0; i < size_; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, handles_[i]);
    }
}

void GLDepthTarget::Bind(uint32_t i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, handles_[i]);
}

GLDepthTarget::~GLDepthTarget() {
    glDeleteTextures(size_, handles_);
}