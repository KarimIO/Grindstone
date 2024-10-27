#pragma once

#include "RenderPass.hpp"
#include "Formats.hpp"
#include "RenderTarget.hpp"
#include "DepthStencilTarget.hpp"
#include <vector>
#include <stdint.h>

namespace Grindstone::GraphicsAPI {
	class RenderPass;
	class RenderTarget;
	class DepthStencilTarget;

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

			RenderTarget** renderTargetLists;
			uint32_t numRenderTargetLists;
			DepthStencilTarget* depthTarget;
			bool isCubemap = false;
		};

		virtual uint32_t GetAttachment(uint32_t attachmentIndex) = 0;
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
		virtual RenderTarget* GetRenderTarget(uint32_t index) const = 0;
		virtual DepthStencilTarget* GetDepthStencilTarget() const = 0;
		virtual ~Framebuffer() {};
	};
}
