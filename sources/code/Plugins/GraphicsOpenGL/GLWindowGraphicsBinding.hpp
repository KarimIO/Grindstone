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
			virtual bool initialize(Window *window) override;
			virtual void immediateSetContext() override;
			virtual void immediateSwapBuffers() override;
			~GLWindowGraphicsBinding();
		public:
			void shareLists(GLWindowGraphicsBinding* binding_to_copy_from);
		private:
			Window* window_;
#ifdef _WIN32
			HWND	window_handle_;
			HGLRC	window_render_context_;
			HDC		window_device_context_;
#endif
		};
	};
};