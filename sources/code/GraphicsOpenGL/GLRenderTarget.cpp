#include "GLRenderTarget.hpp"
#include <GL/gl3w.h>
#include <cmath>
#include "GLTexture.hpp"
#include <../deps/glm/glm.hpp>
#include <iostream>

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
        bool isCompressed;
        TranslateColorFormats(create_info[i].format, isCompressed, format_[i], internalFormat);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, create_info[i].width, create_info[i].height, 0, format_[i], GL_HALF_FLOAT, 0);

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

float GLRenderTarget::getAverageValue(uint32_t i) {
    glBindTexture(GL_TEXTURE_2D, handles_[i]);
    //glGenerateMipmap(GL_TEXTURE_2D);

    unsigned int s = width_ * height_;

    GLfloat *values = new GLfloat[s * 3];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, values);

    float val = 0;
    for (int i = 0; i < s * 3; i+=3) {
        float lum = glm::dot(glm::vec3(values[i], values[i+1], values[i+2]), glm::vec3(0.3, 0.59, 0.11));
        val += std::log2(lum) / float(s);
    }

    delete[] values;

    return val;
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

void GLRenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) {
	glReadBuffer(GL_FRONT_LEFT);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, resx, resy, GL_RGBA, GL_UNSIGNED_BYTE,  data);
}

GLRenderTarget::~GLRenderTarget() {
    glDeleteTextures(size_, handles_);
}