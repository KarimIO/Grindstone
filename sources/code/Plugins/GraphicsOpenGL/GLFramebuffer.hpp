#ifndef _GL_FRAMEBUFFER_H
#define _GL_FRAMEBUFFER_H

#include <Common/Graphics/Framebuffer.hpp>
#include <Common/Graphics/DLLDefs.hpp>
#include "GLRenderTarget.hpp"
#include "GLDepthTarget.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GLFramebuffer : public Framebuffer {
		public:
			GLFramebuffer(CreateInfo&);
			~GLFramebuffer();
			virtual int GetAttachment(int attachmentIndex) override;
			virtual void Clear(ClearMode mask) override;
			virtual void Resize(uint32_t width, uint32_t height) override;
			virtual void CopyFrom(Framebuffer *) override;
			virtual void Bind(bool depth) override;
			virtual void BindWrite(bool depth) override;
			virtual void BindRead() override;
			virtual void BindTextures(int i) override;
			virtual void Unbind() override;
			
			virtual void Blit(uint32_t i, uint32_t x, uint32_t y, uint32_t w, uint32_t h);
		private:
			void CreateFramebuffer();
		private:
			GLuint fbo_ = 0;
			uint32_t numRenderTargetLists = 0;
			uint32_t numTotalRenderTargets = 0;
			GLRenderTarget **renderTargetLists = nullptr;

			GLDepthTarget *depthTarget = nullptr;
		};
	}
}

#endif