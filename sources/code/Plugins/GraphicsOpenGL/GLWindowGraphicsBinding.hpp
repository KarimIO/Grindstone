#pragma once

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <Common/Window/Window.hpp>
#include <Common/Graphics/WindowGraphicsBinding.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class GLWindowGraphicsBinding : public WindowGraphicsBinding {
		public:
			~GLWindowGraphicsBinding();

			// Inherited via WindowGraphicsBinding
			virtual bool Initialize(Window* window) override;
			virtual void ImmediateSetContext() override;
			virtual void ImmediateSwapBuffers() override;
			virtual void AcquireNextImage() override;
			virtual void SubmitCommandBuffer(CommandBuffer* buffers) override;
			virtual void PresentSwapchain() override;
			virtual RenderPass* GetRenderPass() override;
			virtual Framebuffer* GetCurrentFramebuffer() override;
			virtual uint32_t GetCurrentImageIndex() override;
			virtual uint32_t GetMaxFramesInFlight() override;
		public:
			void ShareLists(GLWindowGraphicsBinding* binding_to_copy_from);
		private:
			Window* window;
#ifdef _WIN32
			HWND	windowHandle;
			HGLRC	windowRenderContext;
			HDC		windowDeviceContext;
#endif
		};
	};
};
