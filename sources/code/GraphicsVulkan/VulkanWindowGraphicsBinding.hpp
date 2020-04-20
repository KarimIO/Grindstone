#pragma once

#include <Windows.h>
#include "../GraphicsCommon/WindowGraphicsBinding.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class GLWindowGraphicsBinding : public WindowGraphicsBinding {
		public:
			virtual void initialize(BaseWindow *window) override;
		private:
#ifdef _WIN32
			HGLRC	window_render_context_;
			HDC		window_device_context_;
#endif
		};
	};
};