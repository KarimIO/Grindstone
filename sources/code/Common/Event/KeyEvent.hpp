#pragma once

#include "BaseEvent.hpp"
#include "KeyPressCode.hpp"

namespace Grindstone {
	namespace Events {
		struct KeyPressEvent : public BaseEvent {
			KeyPressEvent(KeyPressCode code, bool isPressed)
				: code(code), isPressed(isPressed) {}
			KeyPressCode code = KeyPressCode::Invalid;
			bool isPressed = false;

			SETUP_EVENT(KeyPress)
		}; // struct KeyPressEvent

		struct CharacterTypedEvent : public BaseEvent {
			CharacterTypedEvent(unsigned short character)
				: character(character) {}
			unsigned short character = 0;

			SETUP_EVENT(CharacterTyped)
		}; // struct CharacterTypedEvent
	} // namespace Events
} // namespace Grindstone
