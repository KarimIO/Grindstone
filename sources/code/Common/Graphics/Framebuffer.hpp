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
			Stencil = 4,
			All = 7
		};

		class RenderPass;

		class Framebuffer {
		public:
			struct DefaultFramebufferCreateInfo {
				uint32_t width;
				uint32_t height;
				RenderPass* renderPass;
			};

			struct CreateInfo {
				RenderPass* renderPass;

				RenderTarget** renderTargetLists;
				uint32_t numRenderTargetLists;
				DepthTarget* depthTarget;
			};

			virtual int GetAttachment(int attachmentIndex) = 0;
			virtual void Resize(uint32_t width, uint32_t height) = 0;
			virtual void Clear(ClearMode mask) = 0;
			virtual void CopyFrom(Framebuffer *) = 0;
			virtual void BindWrite(bool depth) = 0;
			virtual void BindTextures(int i) = 0;
			virtual void Bind(bool depth) = 0;
			virtual void BindRead() = 0;
			virtual void Unbind() = 0;
			virtual ~Framebuffer() {};
		};
	}
}