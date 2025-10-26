#pragma once

#include "Formats.hpp"
#include <stdint.h>

namespace Grindstone::GraphicsAPI {
	/*! Values to clear an attachment with. Use the unions with the correct data type
		that represents the attachment, and use each of the 4 members of the array
		to represent red, green, blue, and alpha respectively.
	*/
	typedef union ClearColor {
		float       float32[4];
		int32_t     int32[4];
		uint32_t    uint32[4];
	} ClearColorValue;

	// Clear values for the DepthStencilBuffer.
	struct ClearDepthStencil {
		bool hasDepthStencilAttachment = false;
		float       depth = 0.f;
		uint32_t    stencil = 0;
	};

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
