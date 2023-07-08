#ifndef _DEPTH_TARGET_H
#define _DEPTH_TARGET_H

#include <stdint.h>
#include "Formats.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class DepthTarget {
		public:
			struct CreateInfo {
				const char* debugName = nullptr;
				DepthFormat format = DepthFormat::None;
				uint32_t width = 0, height = 0;
				bool isShadowMap = false;
				bool isCubemap = false;
				bool isSampled = false;

				CreateInfo() {};
				CreateInfo(DepthFormat depthFormat, uint32_t width, uint32_t height, bool isShadowMap, bool isCubeMap, bool isSampled, const char* debugName) :
					format(depthFormat),
					width(width), height(height),
					isShadowMap(isShadowMap),
					isCubemap(isCubeMap),
					isSampled(isSampled),
					debugName(debugName) {}
			};

			virtual void Resize(uint32_t width, uint32_t height) = 0;
			virtual void BindFace(int k) = 0;
			virtual ~DepthTarget() {};
		};
	};
};

#endif
