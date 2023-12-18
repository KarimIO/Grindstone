#pragma once

#include <Common/Input/InputInterface.hpp>

namespace Grindstone {
	namespace Input {
		class Manager : public Input::Interface {
		public:
			virtual void ResizeEvent(int, int) override;
			virtual void SetMouseButton(MouseAction, ButtonStatus) override;
			virtual ButtonStatus GetMouseButton(MouseAction) override;
			virtual void SetMousePosition(int, int) override;
			virtual void GetMousePosition(int&, int&) override;
			virtual void SetFocused(bool) override;
			virtual bool IsFocused() override;
			virtual void SetKey(KeyAction, ButtonStatus) override;
			virtual ButtonStatus GetKey(KeyAction) override;
			virtual void Quit() override;
			virtual void ForceQuit() override;
		}; // class Manager
	} // namespace Input
} // namespace Grindstone
