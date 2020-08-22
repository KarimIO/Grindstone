#pragma once

#include <Common/Graphics/CommandBuffer.hpp>

namespace Grindstone {
	class Window;

	namespace GraphicsAPI {
		class WindowGraphicsBinding {
		public:
			~WindowGraphicsBinding() {};
			virtual bool initialize(Window *window) = 0;
			virtual void immediateSetContext() {};
			virtual void immediateSwapBuffers() {};
			virtual void presentCommandBuffer(CommandBuffer**buffers, uint32_t num_buffers) {};
		};
	};
};