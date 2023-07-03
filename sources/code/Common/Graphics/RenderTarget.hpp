#ifndef _RENDER_TARGET_H
#define _RENDER_TARGET_H

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {

		class RenderTarget {
		public:
			struct CreateInfo {
				ColorFormat format = ColorFormat::Invalid;
				bool isSampled;
				uint32_t width = 0, height = 0;
				CreateInfo() {};
				CreateInfo(ColorFormat colorFormat, uint32_t width, uint32_t height, bool isSampled) :
					format(colorFormat),
					width(width),
					height(height),
					isSampled(isSampled) {}
			};

			virtual void Resize(uint32_t width, uint32_t height) = 0;
			virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) = 0;
			virtual ~RenderTarget() {};
		};
	}
}

#endif
