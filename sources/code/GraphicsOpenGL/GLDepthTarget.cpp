#include "GLDepthTarget.hpp"
#include <GL/gl3w.h>
#include "GLTexture.hpp"

GLDepthTarget::GLDepthTarget(DepthTargetCreateInfo create_info) {
	glGenTextures(1, &handle_);
    cubemap = create_info.cubemap;
	if (cubemap) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, handle_);

		GLint internalFormat;
		GLenum format;
		TranslateDepthFormats(create_info.format, format, internalFormat);

		for (size_t i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, create_info.width, create_info.height, 0, format, GL_FLOAT, 0);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (create_info.shadow_map) {
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
		glBindTexture(GL_TEXTURE_2D, handle_);

		GLint internalFormat;
		GLenum format;
		TranslateDepthFormats(create_info.format, format, internalFormat);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, create_info.width, create_info.height, 0, format, GL_FLOAT, 0);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		if (create_info.shadow_map) {
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

uint32_t GLDepthTarget::getHandle() {
    return handle_;
}

void GLDepthTarget::Bind(int i) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(cubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, handle_);
}

void GLDepthTarget::BindFace(int k) {
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + k, handle_, 0);
    glDrawBuffer(GL_NONE);
}

GLDepthTarget::~GLDepthTarget() {
    glDeleteTextures(1, &handle_);
}