#include "GLRenderTarget.hpp"
#include <GL/gl3w.h>
#include <cmath>
#include "GLTexture.hpp"
#include <../deps/glm/glm.hpp>
#include <iostream>

namespace Grindstone {
	namespace GraphicsAPI {
		GLRenderTarget::GLRenderTarget(CreateInfo *create_info, uint32_t count, bool cubemap) {
			size_ = count;
			width_ = create_info[0].width;
			height_ = create_info[0].height;
			is_cubemap_ = cubemap;

			handles_ = new uint32_t[size_];
			glGenTextures(size_, handles_);
			format_ = new GLenum[size_];

			if (cubemap) {
				for (uint32_t i = 0; i < count; i++) {
					glBindTexture(GL_TEXTURE_CUBE_MAP, handles_[i]);

					GLint internalFormat;
					bool isCompressed;
					translateColorFormats(create_info[i].format, isCompressed, format_[i], internalFormat);

					for (unsigned int f = 0; f < 6u; ++f) {
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, internalFormat, create_info[i].width, create_info[i].height, 0, format_[i], GL_FLOAT, nullptr);
					}

					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR_MIPMAP_LINEAR
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
			}
			else {
				for (uint32_t i = 0; i < count; i++) {
					glBindTexture(GL_TEXTURE_2D, handles_[i]);

					GLint internalFormat;
					bool isCompressed;
					translateColorFormats(create_info[i].format, isCompressed, format_[i], internalFormat);

					glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, create_info[i].width, create_info[i].height, 0, format_[i], GL_FLOAT, 0);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				}
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
			glBindTexture(is_cubemap_ ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, handles_[i]);
			glGenerateMipmap(is_cubemap_ ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D);

			unsigned int w = 8;
			unsigned int s = w * w;

			GLfloat *values = new GLfloat[s];
			//glGetTexImage(GL_TEXTURE_2D, 9, GL_RED, GL_FLOAT, values);
			glReadPixels(0, 0, w, w, GL_RED, GL_FLOAT, values);
			float val = 0;
			for (uint32_t i = 0; i < s; i++) {
				float lum = values[i];
				val += lum;
			}

			val /= float(s);

			delete[] values;

			return val;
		}

		void GLRenderTarget::Bind() {
			for (uint32_t i = 0; i < size_; i++) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(is_cubemap_ ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, handles_[i]);
			}
		}

		void GLRenderTarget::Bind(uint32_t j) {
			for (uint32_t i = 0; i < size_; i++) {
				glActiveTexture(GL_TEXTURE0 + j + i);
				glBindTexture(is_cubemap_ ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, handles_[i]);
			}
		}

		void GLRenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) {
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadPixels(0, 0, resx, resy, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		GLRenderTarget::~GLRenderTarget() {
			glDeleteTextures(size_, handles_);
		}
	}
}