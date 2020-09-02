#pragma once

#include <Common/Input.hpp>

namespace Grindstone {
	namespace Input {
		class Manager : public Input::Interface {
		public:
			virtual void resizeEvent(int, int) override;
			virtual void setMouseButton(MouseAction, ButtonStatus) override;
			virtual ButtonStatus getMouseButton(MouseAction) override;
			virtual void setMousePosition(int, int) override;
			virtual void getMousePosition(int&, int&) override;
			virtual void setFocused(bool) override;
			virtual bool isFocused() override;
			virtual void setKey(KeyAction, ButtonStatus) override;
			virtual ButtonStatus getKey(KeyAction) override;
			virtual void quit() override;
			virtual void forceQuit() override;
		}; // class Manager
	} // namespace Input
} // namespace Grindstone
