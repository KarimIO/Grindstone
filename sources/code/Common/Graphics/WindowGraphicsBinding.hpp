#pragma once

#include <Common/Graphics/Formats.hpp>

namespace Grindstone {
	class Window;

	namespace GraphicsAPI {
		class CommandBuffer;
		class RenderPass;
		class RenderTarget;
		class Framebuffer;

		class WindowGraphicsBinding {
		public:
			~WindowGraphicsBinding() {};
			virtual bool Initialize(Window *window) = 0;
			virtual void ImmediateSetContext() = 0;
			virtual void ImmediateSwapBuffers() = 0;
			virtual bool AcquireNextImage() = 0;
			virtual void SubmitCommandBuffer(CommandBuffer* buffers) = 0;
			virtual bool PresentSwapchain() = 0;
			virtual Grindstone::GraphicsAPI::ColorFormat GetSwapchainFormat() const = 0;
			virtual Grindstone::GraphicsAPI::RenderTarget* GetSwapchainRenderTarget(uint32_t i) = 0;
			virtual uint32_t GetCurrentImageIndex() const = 0;
			virtual uint32_t GetMaxFramesInFlight() const = 0;
			virtual void Resize(uint32_t width, uint32_t height) = 0;
		};
	};
};
