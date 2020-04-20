#pragma once

#include "../WindowModule/BaseWindow.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class WindowGraphicsBinding {
		public:
			virtual void initialize(BaseWindow *window) = 0;
		};
	};
};