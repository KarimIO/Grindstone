#pragma once

#include "Formats.hpp"
#include <stdint.h>

namespace Grindstone::GraphicsAPI {
	/*! A RenderPass describes the formats to be used by one or more GraphicsPipeline.
		Framebuffers will then be based on a RenderPass.
	*/
	class RenderPass {
	public:
		struct AttachmentInfo {
			Format colorFormat = Format::Invalid;
			bool shouldClear = false;
			GraphicsAPI::MemoryUsage memoryUsage = GraphicsAPI::MemoryUsage::GPUOnly;
		};

		struct CreateInfo {
			const char* debugName = nullptr;
			AttachmentInfo* colorAttachments = nullptr;
			uint32_t colorAttachmentCount = 0;
			Format depthFormat = Format::Invalid;
			bool shouldClearDepthOnLoad = true;
			float debugColor[4];
		};

		virtual const char* GetDebugName() const = 0;
		virtual const float* GetDebugColor() const = 0;
		virtual ~RenderPass() {};
	};
}
