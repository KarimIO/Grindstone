#pragma once

#include "Event/KeyPressCode.hpp"
#include "Event/MouseButtonCode.hpp"

namespace Grindstone {
	namespace Input {
		class Interface {
		public:
			virtual void ResizeEvent(int, int) = 0;
			virtual void SetMouseButton(Events::MouseButtonCode, bool) = 0;
			virtual bool IsMouseButtonPressed(Events::MouseButtonCode) = 0;
			virtual void SetMousePosition(int, int) = 0;
			virtual void GetMousePosition(int&, int&) = 0;
			virtual void SetIsFocused(bool) = 0;
			virtual bool IsFocused() = 0;
			virtual void MouseScroll(int offsetX, int offsetY) = 0;
			virtual void SetKeyPressed(Events::KeyPressCode, bool) = 0;
			virtual bool IsKeyPressed(Events::KeyPressCode) = 0;
			virtual void Quit() = 0;
			virtual void ForceQuit() = 0;
		}; // class Interface
	} // namespace Input
} // namespace Grindstone
