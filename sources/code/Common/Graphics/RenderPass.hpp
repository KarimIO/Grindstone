#pragma once

#include "Framebuffer.hpp"
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
			bool hasDepthStencilAttachment;
			float       depth;
			uint32_t    stencil;
		};

		class RenderPass {
		public:
			struct CreateInfo {
				uint32_t width_;
				uint32_t height_;
				ColorFormat* color_formats_;
				uint32_t color_format_count_;
				DepthFormat depth_format_;
			};

			virtual ~RenderPass() {};
		};
	}
}