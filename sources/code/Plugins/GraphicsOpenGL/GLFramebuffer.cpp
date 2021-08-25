#include <GL/gl3w.h>
#include <iostream>
#include "GLFramebuffer.hpp"
#include "GLTexture.hpp"
#include <cmath>
#include <../deps/glm/glm.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		GLFramebuffer::GLFramebuffer(CreateInfo& createInfo) {
			if (createInfo.debugName != nullptr) {
				debugName = createInfo.debugName;
			}

			renderTargetLists = new GLRenderTarget*[createInfo.numRenderTargetLists];
			numRenderTargetLists = createInfo.numRenderTargetLists;
			depthTarget = (GLDepthTarget *)createInfo.depthTarget;

			numTotalRenderTargets = 0;
			for (uint32_t i = 0; i < createInfo.numRenderTargetLists; i++) {
				renderTargetLists[i] = (GLRenderTarget*)createInfo.renderTargetLists[i];
				numTotalRenderTargets += static_cast<GLRenderTarget *>(createInfo.renderTargetLists[i])->GetNumRenderTargets();
			}

			CreateFramebuffer();
		}
		
		void GLFramebuffer::CreateFramebuffer() {
			if (framebuffer) {
				// glDeleteFramebuffers(1, &framebuffer);
				framebuffer = 0;
			}

			glGenFramebuffers(1, &framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			if (!debugName.empty()) {
				glObjectLabel(GL_FRAMEBUFFER, framebuffer, -1, debugName.c_str());
			}

			GLenum *DrawBuffers = new GLenum[numTotalRenderTargets];
			GLenum k = 0;
			for (uint32_t i = 0; i < numRenderTargetLists; i++) {
				GLRenderTarget *render_target_list = static_cast<GLRenderTarget *>(renderTargetLists[i]);
				for (uint32_t j = 0; j < render_target_list->GetNumRenderTargets(); j++) {
					DrawBuffers[k] = GL_COLOR_ATTACHMENT0 + k;
					if (render_target_list->IsCubemap()) {
						for (int f = 0; f < 6; ++f) {
							glFramebufferTexture2D(GL_FRAMEBUFFER, DrawBuffers[k], GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, render_target_list->GetHandle(j), 0);
						}
						k++;
					}
					else {
						glFramebufferTexture2D(GL_FRAMEBUFFER, DrawBuffers[k++], GL_TEXTURE_2D, render_target_list->GetHandle(j), 0);
					}
				}
			}

			if (depthTarget) {
				GLDepthTarget *dt = static_cast<GLDepthTarget *>(depthTarget);
				if (!dt->IsCubemap())
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dt->GetHandle(), 0);
				else
					for (int f = 0; f < 6; ++f) {
						glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, dt->GetHandle(), 0);
					}
			}

			if (numTotalRenderTargets > 0)
				glDrawBuffers(numTotalRenderTargets, DrawBuffers);
			else {
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}

			//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				//throw std::runtime_error("Framebuffer Incomplete\n");

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			delete[] DrawBuffers;
		}

		GLFramebuffer::~GLFramebuffer() {
			if (framebuffer != 0) {
				glDeleteFramebuffers(1, &framebuffer);
			}
		}

		uint32_t GLFramebuffer::GetAttachment(uint32_t attachmentIndex) {
			return renderTargetLists[0]->GetHandle(attachmentIndex);
		}
		
		void GLFramebuffer::Resize(uint32_t width, uint32_t height) {
			for (uint32_t i = 0; i < numRenderTargetLists; ++i) {
				RenderTarget* renderTargetList = renderTargetLists[i];
				renderTargetList->Resize(width, height);
			}

			if (depthTarget) {
				depthTarget->Resize(width, height);
			}

			CreateFramebuffer();
		}

		void GLFramebuffer::Clear(ClearMode mask) {
			int m = (((uint8_t)mask & (uint8_t)ClearMode::Depth) != 0) ? GL_DEPTH_BUFFER_BIT : 0;
			m = m | ((((uint8_t)mask & (uint8_t)ClearMode::Color) != 0) ? GL_COLOR_BUFFER_BIT : 0);
			glClear(m);
		}

		void GLFramebuffer::Blit(uint32_t i, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
			glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
			glBlitFramebuffer(x, y, x + w, y + h, x, y, x + w, y + h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		void GLFramebuffer::CopyFrom(Framebuffer *) {

		}

		void GLFramebuffer::Bind(bool depth) {
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glDepthMask(depth ? GL_TRUE : GL_FALSE);
		}

		void GLFramebuffer::BindWrite(bool depth) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
			glDepthMask(depth ? GL_TRUE : GL_FALSE);
		}

		void GLFramebuffer::BindRead() {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
		}

		void GLFramebuffer::BindTextures(int k) {
			int j = k;
			for (unsigned int i = 0; i < numRenderTargetLists; i++) {
				renderTargetLists[i]->Bind(j);
				j += renderTargetLists[i]->GetNumRenderTargets();
			}

			if (depthTarget)
				depthTarget->Bind(j);
		}

		void GLFramebuffer::Unbind() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
}