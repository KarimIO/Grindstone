#pragma once

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <Common/Window/Window.hpp>
#include <Common/Graphics/WindowGraphicsBinding.hpp>

namespace Grindstone::GraphicsAPI::OpenGL {
	class WindowGraphicsBinding : public Grindstone::GraphicsAPI::WindowGraphicsBinding {
	public:
		~WindowGraphicsBinding();

		// Inherited via WindowGraphicsBinding
		virtual bool Initialize(Window* newWindow) override;
		virtual void ImmediateSetContext() override;
		virtual void ImmediateSwapBuffers() override;
		virtual bool AcquireNextImage() override;
		virtual void SubmitCommandBuffer(CommandBuffer* buffers) override;
		virtual bool PresentSwapchain() override;
		virtual Grindstone::GraphicsAPI::RenderPass* GetRenderPass() override;
		virtual Grindstone::GraphicsAPI::Framebuffer* GetCurrentFramebuffer() override;
		virtual uint32_t GetCurrentImageIndex() override;
		virtual uint32_t GetMaxFramesInFlight() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
	public:
		void ShareLists(WindowGraphicsBinding* bindingToCopyFrom);
	private:
		Window* window;
#ifdef _WIN32
		HWND	windowHandle;
		HGLRC	windowRenderContext;
		HDC		windowDeviceContext;
#endif
	};
}
