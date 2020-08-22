#ifndef _DEPTH_TARGET_H
#define _DEPTH_TARGET_H

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {

		class DepthTarget {
		public:
			struct CreateInfo {
				DepthFormat format;
				uint32_t width, height;
				bool shadow_map;
				bool cubemap;
				CreateInfo() {};
				CreateInfo(DepthFormat fmt, uint32_t w, uint32_t h, bool s, bool c) : format(fmt), width(w), height(h), shadow_map(s), cubemap(c) {}
			};

			virtual void BindFace(int k) = 0;
			virtual ~DepthTarget() {};
		};
	};
};

#endif