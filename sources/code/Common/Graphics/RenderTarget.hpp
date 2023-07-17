#ifndef _RENDER_TARGET_H
#define _RENDER_TARGET_H

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {

		class RenderTarget {
		public:
			struct CreateInfo {
				const char* debugName = nullptr;
				ColorFormat format = ColorFormat::Invalid;
				bool isSampled = false;
				bool isWrittenByCompute = false;
				bool hasMipChain = false;
				uint32_t width = 0, height = 0;
				CreateInfo() {};
				CreateInfo(ColorFormat colorFormat, uint32_t width, uint32_t height, bool isSampled, bool isWrittenByCompute, const char* name, bool hasMipChain = false) :
					format(colorFormat),
					width(width),
					height(height),
					isSampled(isSampled),
					isWrittenByCompute(isWrittenByCompute),
					debugName(name),
					hasMipChain(hasMipChain) {}
			};

			virtual void Resize(uint32_t width, uint32_t height) = 0;
			virtual void RenderScreen(unsigned int i, unsigned int resx, unsigned int resy, unsigned char *data) = 0;
			virtual ~RenderTarget() {};
		};
	}
}

#endif
