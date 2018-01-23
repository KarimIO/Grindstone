#include "GLDepthTarget.hpp"
#include <GL/gl3w.h>
#include "GLTexture.hpp"

GLDepthTarget::GLDepthTarget(DepthTargetCreateInfo create_info) {
	glGenTextures(1, &handle_);
    
    glBindTexture(GL_TEXTURE_2D, handle_);

    GLint internalFormat;
    GLenum format;
    TranslateDepthFormats(create_info.format, format, internalFormat);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, create_info.width, create_info.height, 0, format, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

uint32_t GLDepthTarget::getHandle() {
    return handle_;
}

void GLDepthTarget::Bind(int i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, handle_);
}

GLDepthTarget::~GLDepthTarget() {
    glDeleteTextures(1, &handle_);
}