#pragma once

#include "Common/Event/KeyPressCode.hpp"
#include "Common/Event/MouseButtonCode.hpp"
#include "Common/Input.hpp"

namespace Grindstone {
	namespace Events {
		class Dispatcher;
	}

	namespace Input {
		class Manager : public Interface {
		public:
			Manager(Events::Dispatcher* dispatcher);
			virtual bool IsMouseButtonPressed(Events::MouseButtonCode) override;
			virtual void GetMousePosition(int&, int&) override;
			virtual bool IsFocused() override;
			virtual bool IsKeyPressed(Events::KeyPressCode) override;

			virtual void ResizeEvent(int, int) override;
			virtual void SetMouseButton(Events::MouseButtonCode, bool) override;
			virtual void SetMousePosition(int, int) override;
			virtual void SetIsFocused(bool) override;
			virtual void MouseScroll(float, float) override;
			virtual void SetKeyPressed(Events::KeyPressCode, bool) override;
			virtual void AddCharacterTyped(unsigned short character) override;
			virtual void Quit() override;
			virtual void ForceQuit() override;
		private:
			Events::Dispatcher* dispatcher = nullptr;

			bool isFocused = false;
			int mousePositionX = 0;
			int mousePositionY = 0;
			bool keyPressed[(int)Events::KeyPressCode::Last];
			bool mousePressed[(int)Events::MouseButtonCode::Last];
		}; // class Manager
	} // namespace Input
} // namespace Grindstone
