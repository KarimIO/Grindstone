#ifndef _GL_RENDER_TARGET_H
#define _GL_RENDER_TARGET_H

#include <stdint.h>
#include <Common/Graphics/RenderTarget.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLRenderTarget : public RenderTarget {
		public:
			GLRenderTarget(CreateInfo& createInfoList);
			GLRenderTarget(CreateInfo *createInfoList, uint32_t createInfoCount, bool isCcubemap);
			virtual uint32_t GetHandle();
			virtual uint32_t GetHandle(uint32_t i);
			uint32_t GetNumRenderTargets();
			virtual void Resize(uint32_t width, uint32_t height);

			bool IsCubemap();
			virtual void Bind();
			virtual void Bind(uint32_t i);
			virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data);
			virtual ~GLRenderTarget();
		private:
			void CreateRenderTargets();
		private:
			bool isCubemap;
			int *internalFormats;
			unsigned int *formats;
			uint32_t *renderTargetHandles;
			uint32_t renderTargetCount;
			uint32_t width, height;
		};
	}
}

#endif
