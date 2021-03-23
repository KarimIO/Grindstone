#pragma once

#include "BaseEvent.hpp"
#include "KeyPressCode.hpp"

namespace Grindstone {
	namespace Events {
		struct KeyPressEvent : public BaseEvent {
			KeyPressEvent(KeyPressCode code, bool isPressed)
				: code(code), isPressed(isPressed) {}
			KeyPressCode code = KeyPressCode::None;
			bool isPressed = false;

			SETUP_EVENT(KeyPress)
		}; // struct KeyPressEvent
		
		struct CharacterTypedEvent : public BaseEvent {
			CharacterTypedEvent(KeyPressCode code)
				: code(code) {}
			KeyPressCode code = KeyPressCode::None;

			SETUP_EVENT(CharacterTyped)
		}; // struct CharacterTypedEvent
	} // namespace Events
} // namespace Grindstone
