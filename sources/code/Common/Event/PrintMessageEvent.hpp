#pragma once

#include <string>

#include "Common/Logging.hpp"

#include "BaseEvent.hpp"
#include "KeyPressCode.hpp"

namespace Grindstone::Events {
		struct PrintMessageEvent : public BaseEvent {
		PrintMessageEvent(ConsoleMessage msg)
			: message(msg) {}
		ConsoleMessage message;

		SETUP_EVENT(PrintMessage)
	}; // struct PrintMessageEvent
} // namespace Grindstone::Events
