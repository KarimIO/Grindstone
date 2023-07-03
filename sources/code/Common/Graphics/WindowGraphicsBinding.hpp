#pragma once

namespace Grindstone {
	class Window;

	namespace GraphicsAPI {
		class CommandBuffer;
		class RenderPass;
		class Framebuffer;

		class WindowGraphicsBinding {
		public:
			~WindowGraphicsBinding() {};
			virtual bool Initialize(Window *window) = 0;
			virtual void ImmediateSetContext() {};
			virtual void ImmediateSwapBuffers() {};
			virtual void PresentCommandBuffer(CommandBuffer**buffers, uint32_t num_buffers) {};
			virtual RenderPass* GetRenderPass() { return nullptr; };
			virtual Framebuffer* GetCurrentFramebuffer() { return nullptr; };
		};
	};
};
