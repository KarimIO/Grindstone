#pragma once

#include <Common/Graphics/Formats.hpp>

namespace Grindstone {
	class Window;
}

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class RenderPass;
	class Framebuffer;
	class Image;

	/*! These are used to encapsulate functionality that ties a windowing system to
		a graphics API. They are used to acquire images, present images, and get
		the swap chain.
	*/
	class WindowGraphicsBinding {
	public:
		~WindowGraphicsBinding() {};
		virtual bool Initialize(Window *window) = 0;
		virtual void WaitForRenderingFence() = 0;
		virtual void ImmediateSetContext() = 0;
		virtual void ImmediateSwapBuffers() = 0;
		virtual bool AcquireNextImage() = 0;
		virtual void SubmitCommandBufferNoSynchronization(GraphicsAPI::CommandBuffer* buffer) = 0;
		virtual void SubmitCommandBufferForCurrentFrame(GraphicsAPI::CommandBuffer* buffer) = 0;
		virtual bool PresentSwapchain() = 0;
		virtual RenderPass* GetRenderPass() const = 0;
		virtual Framebuffer* GetCurrentFramebuffer() const = 0;
		virtual Image* GetCurrentSwapchainImage() const = 0;
		virtual Image* GetSwapchainImage(uint32_t index) const = 0;
		virtual uint32_t GetCurrentSwapchainIndex() const = 0;
		virtual uint32_t GetCurrentImageIndex() const = 0;
		virtual uint32_t GetCurrentFrame() const = 0;
		virtual uint32_t GetMaxFramesInFlight() const = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual GraphicsAPI::Format GetSwapchainFormat() const = 0;
	};
};
