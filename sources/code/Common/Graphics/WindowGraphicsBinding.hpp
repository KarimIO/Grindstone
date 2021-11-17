#pragma once

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
	}

	class Window;

	namespace GraphicsAPI {
		class WindowGraphicsBinding {
		public:
			~WindowGraphicsBinding() {};
			virtual bool Initialize(Window *window) = 0;
			virtual void ImmediateSetContext() {};
			virtual void ImmediateSwapBuffers() {};
			virtual void PresentCommandBuffer(CommandBuffer**buffers, uint32_t num_buffers) {};
		};
	};
};
