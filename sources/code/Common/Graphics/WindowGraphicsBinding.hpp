#pragma once

namespace Grindstone {
	class Window;
}

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class RenderPass;
	class Framebuffer;

	/*! These are used to encapsulate functionality that ties a windowing system to
		a graphics API. They are used to acquire images, present images, and get
		the swap chain.
	*/
	class WindowGraphicsBinding {
	public:
		~WindowGraphicsBinding() {};
		virtual bool Initialize(Window *window) = 0;
		virtual void ImmediateSetContext() = 0;
		virtual void ImmediateSwapBuffers() = 0;
		virtual bool AcquireNextImage() = 0;
		virtual void SubmitCommandBufferNoSynchronization(GraphicsAPI::CommandBuffer* buffer) = 0;
		virtual void SubmitCommandBufferForCurrentFrame(GraphicsAPI::CommandBuffer* buffer) = 0;
		virtual bool PresentSwapchain() = 0;
		virtual RenderPass* GetRenderPass() = 0;
		virtual Framebuffer* GetCurrentFramebuffer() = 0;
		virtual uint32_t GetCurrentImageIndex() = 0;
		virtual uint32_t GetMaxFramesInFlight() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
	};
};
