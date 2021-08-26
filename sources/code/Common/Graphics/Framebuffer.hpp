#pragma once

#include "RenderPass.hpp"
#include "Formats.hpp"
#include "RenderTarget.hpp"
#include "DepthTarget.hpp"
#include <vector>
#include <stdint.h>


namespace Grindstone {
	namespace GraphicsAPI {
		enum class ClearMode : uint8_t {
			Color = 1,
			Depth = 2,
			ColorAndDepth = 3,
			Stencil = 4,
			All = 7
		};

		class RenderPass;

		class Framebuffer {
		public:
			struct DefaultFramebufferCreateInfo {
				uint32_t width;
				uint32_t height;
				RenderPass* renderPass = nullptr;
			};

			struct CreateInfo {
				const char* debugName;
				RenderPass* renderPass;

				RenderTarget** renderTargetLists;
				uint32_t numRenderTargetLists;
				DepthTarget* depthTarget;
			};

			virtual uint32_t GetAttachment(uint32_t attachmentIndex) = 0;
			virtual void Resize(uint32_t width, uint32_t height) = 0;
			virtual void Clear(ClearMode mask) = 0;
			virtual void CopyFrom(Framebuffer *) = 0;
			virtual void BindTextures(int i) = 0;
			virtual void Bind() = 0;
			virtual void BindWrite() = 0;
			virtual void BindRead() = 0;
			virtual void Unbind() = 0;
			virtual ~Framebuffer() {};
		};
	}
}