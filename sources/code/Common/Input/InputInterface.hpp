#pragma once

#include <Common/Event/KeyPressCode.hpp>
#include <Common/Event/MouseButtonCode.hpp>
#include <Common/Input/CursorMode.hpp>

namespace Grindstone {
	class Window;

	namespace Input {
		class Interface {
		public:
			virtual CursorMode GetCursorMode() = 0;
			virtual void SetCursorMode(CursorMode) = 0;
			virtual void SetCursorIsRawMotion(bool isRawMode) = 0;
			virtual bool GetCursorIsRawMotion() = 0;
			virtual void SetMainWindow(Grindstone::Window* window) = 0;
			virtual void ResizeEvent(int, int) = 0;
			virtual void SetMouseButton(Events::MouseButtonCode, bool) = 0;
			virtual bool IsMouseButtonPressed(Events::MouseButtonCode) = 0;
			virtual void SetMousePosition(int, int) = 0;
			virtual void GetMousePosition(int&, int&) = 0;
			virtual void OnMouseMoved(int, int) = 0;
			virtual void SetIsFocused(bool) = 0;
			virtual bool IsFocused() = 0;
			virtual void MouseScroll(float offsetX, float offsetY) = 0;
			virtual void SetKeyPressed(Events::KeyPressCode, bool) = 0;
			virtual bool IsKeyPressed(Events::KeyPressCode) = 0;
			virtual void AddCharacterTyped(unsigned short character) = 0;
			virtual void TryQuit(Grindstone::Window* window) = 0;
			virtual void ForceQuit(Grindstone::Window* window) = 0;
		}; // class Interface
	} // namespace Input
} // namespace Grindstone
