#pragma once

#include "InputCodes.hpp"

namespace Grindstone {
	namespace Input {
		class Interface {
		public:
			virtual void resizeEvent(int, int) = 0;
			virtual void setMouseButton(MouseAction, ButtonStatus) = 0;
			virtual ButtonStatus getMouseButton(MouseAction) = 0;
			virtual void setMousePosition(int, int) = 0;
			virtual void getMousePosition(int&, int&) = 0;
			virtual void setFocused(bool) = 0;
			virtual bool isFocused() = 0;
			virtual void setKey(KeyAction, ButtonStatus) = 0;
			virtual ButtonStatus getKey(KeyAction) = 0;
			virtual void quit() = 0;
			virtual void forceQuit() = 0;
		}; // class Interface
	} // namespace Input
} // namespace Grindstone
