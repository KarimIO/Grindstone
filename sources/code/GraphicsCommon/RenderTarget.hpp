#ifndef _RENDER_TARGET_H
#define _RENDER_TARGET_H

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		struct RenderTargetCreateInfo {
			ColorFormat format;
			uint32_t width, height;
			RenderTargetCreateInfo() {};
			RenderTargetCreateInfo(ColorFormat fmt, uint32_t w, uint32_t h) : format(fmt), width(w), height(h) {}
		};

		class RenderTarget {
		public:
			virtual float getAverageValue(uint32_t i) = 0;
			virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) = 0;
			virtual ~RenderTarget() {};
		};
	}
}

#endif