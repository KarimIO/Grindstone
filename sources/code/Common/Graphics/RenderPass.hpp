#pragma once

#include "Formats.hpp"
#include <stdint.h>


namespace Grindstone {
	namespace GraphicsAPI {
		typedef union ClearColor {
			float       float32[4];
			int32_t     int32[4];
			uint32_t    uint32[4];
		} ClearColorValue;

		struct ClearDepthStencil {
			bool hasDepthStencilAttachment = false;
			float       depth = 0.f;
			uint32_t    stencil = 0;
		};

		class RenderPass {
		public:
			struct CreateInfo {
				const char* debugName = nullptr;
				uint32_t width = 0;
				uint32_t height = 0;
				ColorFormat* colorFormats = nullptr;
				uint32_t colorFormatCount = 0;
				DepthFormat depthFormat = DepthFormat::None;
			};

			virtual void Resize(uint32_t width, uint32_t height) = 0;
			virtual uint32_t GetWidth() = 0;
			virtual uint32_t GetHeight() = 0;
			virtual ~RenderPass() {};
		};
	}
}
