#pragma once

#include "RenderPass.hpp"
#include "Formats.hpp"
#include <vector>
#include <stdint.h>

namespace Grindstone::GraphicsAPI {
	class RenderPass;
	class Image;

	/*! A framebuffer is a series of one or more RenderTarget and optionally a
		DepthStencilTarget. Framebuffers bind these attachments together to be drawn to
		by a GraphicsPipeline.
	*/
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
			uint32_t width;
			uint32_t height;

			Image** renderTargets;
			uint32_t renderTargetCount;
			Image* depthTarget;
			bool isCubemap = false;
		};

		virtual RenderPass* GetRenderPass() const = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Clear(ClearMode mask) = 0;
		virtual void BindTextures(int i) = 0;
		virtual void Bind() = 0;
		virtual void BindWrite() = 0;
		virtual void BindRead() = 0;
		virtual void Unbind() = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRenderTargetCount() const = 0;
		virtual Image* GetRenderTarget(uint32_t index) const = 0;
		virtual Image* GetDepthStencilTarget() const = 0;
		virtual ~Framebuffer() {};
	};
}
