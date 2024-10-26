#pragma once

#include <stdint.h>

#include "Formats.hpp"

namespace Grindstone::GraphicsAPI {
	/*! Depth Targets are a kind of image used as an attachment in one or more
		GraphicsPipeline. They are meant to contain both depth and stencil data.
		Depth data refers to how far pixels are from the screen, and are used to
		order which triangles appear on top when drawing many triangles to the
		screen. Stencil buffers are arbitrary data placed alongside the depth target,
		often used for controlling where rendering occurs.
	*/
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
}
