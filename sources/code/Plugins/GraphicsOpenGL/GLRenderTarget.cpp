#include "GLRenderTarget.hpp"
#include <GL/gl3w.h>
#include <cmath>
#include "GLTexture.hpp"
#include <../deps/glm/glm.hpp>
#include <iostream>

namespace Grindstone {
	namespace GraphicsAPI {
		GLRenderTarget::GLRenderTarget(CreateInfo* createInfoList, uint32_t createInfoCount, bool isCubemap) {
			renderTargetCount = createInfoCount;
			width = std::max(createInfoList[0].width, 1u);
			height = std::max(createInfoList[0].height, 1u);
			this->isCubemap = isCubemap;

			renderTargetHandles = new uint32_t[renderTargetCount];
			formats = new GLenum[renderTargetCount];
			internalFormats = new GLint[renderTargetCount];

			for (uint32_t i = 0; i < renderTargetCount; i++) {
				bool isCompressed;
				translateColorFormats(createInfoList[i].format, isCompressed, formats[i], internalFormats[i]);
			}

			CreateRenderTargets();
		}

		void GLRenderTarget::CreateRenderTargets() {
			if (renderTargetHandles[0] > 0) {
				glDeleteTextures(renderTargetCount, renderTargetHandles);

				for (uint32_t i = 0; i < renderTargetCount; i++) {
					renderTargetHandles[i] = 0;
				}
			}

			glGenTextures(renderTargetCount, renderTargetHandles);
			if (isCubemap) {
				for (uint32_t i = 0; i < renderTargetCount; i++) {
					glBindTexture(GL_TEXTURE_CUBE_MAP, renderTargetHandles[i]);

					for (unsigned int f = 0; f < 6u; ++f) {
						glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, 0, internalFormats[i], width, height, 0, formats[i], GL_FLOAT, nullptr);
					}

					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR_MIPMAP_LINEAR
					glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				}
			}
			else {
				for (uint32_t i = 0; i < renderTargetCount; i++) {
					glBindTexture(GL_TEXTURE_2D, renderTargetHandles[i]);

					glTexImage2D(GL_TEXTURE_2D, 0, internalFormats[i], width, height, 0, formats[i], GL_FLOAT, 0);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				}
			}
		}

		uint32_t GLRenderTarget::GetHandle() {
			return renderTargetHandles[0];
		}

		uint32_t GLRenderTarget::GetHandle(uint32_t i) {
			return renderTargetHandles[i];
		}

		uint32_t GLRenderTarget::GetNumRenderTargets() {
			return renderTargetCount;
		}

		bool GLRenderTarget::IsCubemap() {
			return isCubemap;
		}
		
		void GLRenderTarget::Resize(uint32_t width, uint32_t height) {
			this->width = width;
			this->height = height;

			CreateRenderTargets();
		}

		void GLRenderTarget::Bind() {
			for (uint32_t i = 0; i < renderTargetCount; i++) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, renderTargetHandles[i]);
			}
		}

		void GLRenderTarget::Bind(uint32_t j) {
			for (uint32_t i = 0; i < renderTargetCount; i++) {
				glActiveTexture(GL_TEXTURE0 + j + i);
				glBindTexture(isCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D, renderTargetHandles[i]);
			}
		}

		void GLRenderTarget::RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) {
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glReadPixels(0, 0, resx, resy, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		GLRenderTarget::~GLRenderTarget() {
			glDeleteTextures(renderTargetCount, renderTargetHandles);
		}
	}
}