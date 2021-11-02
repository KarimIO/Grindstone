#pragma once

namespace Grindstone {
	namespace Events {
		enum class EventType {
			WindowTryQuit = 0,
			WindowForceQuit,
			WindowResize,
			WindowStartFocus,
			WindowKillFocus,
			WindowMoved,

			KeyPress,
			CharacterTyped,

			MouseButton,
			MouseMoved,
			MouseScrolled,
			PrintMessage,
			Last
		}; // enum class EventType
	} // namespace Events
} // namespace Grindstone
