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
			struct AttachmentInfo {
				ColorFormat colorFormat = ColorFormat::Invalid;
				bool shouldClear = false;
			};

			struct CreateInfo {
				const char* debugName = nullptr;
				AttachmentInfo* colorAttachments = nullptr;
				uint32_t colorAttachmentCount = 0;
				DepthFormat depthFormat = DepthFormat::None;
				bool shouldClearDepthOnLoad = true;
				float debugColor[4];
			};

			virtual const char* GetDebugName() const = 0;
			virtual const float* GetDebugColor() const = 0;
			virtual ~RenderPass() {};
		};
	}
}
