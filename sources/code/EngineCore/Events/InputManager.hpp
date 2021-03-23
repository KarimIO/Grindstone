#pragma once

#include "Common/Event/KeyPressCode.hpp"
#include "Common/Event/MouseButtonCode.hpp"
#include "Common/Input.hpp"

namespace Grindstone {
	namespace Input {
		class Manager : public Interface {
		public:
			virtual bool IsMouseButtonPressed(Events::MouseButtonCode) override;
			virtual void GetMousePosition(int&, int&) override;
			virtual bool IsFocused() override;
			virtual bool IsKeyPressed(Events::KeyPressCode) override;

			virtual void ResizeEvent(int, int) override;
			virtual void SetMouseButton(Events::MouseButtonCode, bool) override;
			virtual void SetMousePosition(int, int) override;
			virtual void SetIsFocused(bool) override;
			virtual void MouseScroll(int, int) override;
			virtual void SetKeyPressed(Events::KeyPressCode, bool) override;
			virtual void Quit() override;
			virtual void ForceQuit() override;
		}; // class Manager
	} // namespace Input
} // namespace Grindstone
