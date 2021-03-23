#pragma once

#include "BaseEvent.hpp"
#include "MouseButtonCode.hpp"

namespace Grindstone {
	namespace Events {
		struct MousePressEvent : public BaseEvent {
			MousePressEvent(MouseButtonCode code, bool isPressed)
				: code(code), isPressed(isPressed) {}
			MouseButtonCode code = MouseButtonCode::None;
			bool isPressed = false;

			SETUP_EVENT(MouseButton)
		}; // struct MousePressEvent
		
		struct MouseMovedEvent : public BaseEvent {
			MouseMovedEvent(int mouseX, int mouseY)
				: mouseX(mouseX), mouseY(mouseY) {}
			int mouseX;
			int mouseY;

			SETUP_EVENT(MouseMoved)
		}; // struct MouseMovedEvent
		
		struct MouseScrolledEvent : public BaseEvent {
			MouseScrolledEvent(int scrollX, int scrollY)
				: scrollX(scrollX), scrollY(scrollY) {}
			int scrollX;
			int scrollY;

			SETUP_EVENT(MouseScrolled)
		}; // struct MouseScrolledEvent
	} // namespace Events
} // namespace Grindstone
