#pragma once

namespace Grindstone {
	namespace Events {
		enum class EventType {
			WindowClose = 0,
			WindowResize,
			WindowStartFocus,
			WindowKillFocus,
			WindowMoved,

			KeyPress,
			CharacterTyped,

			MouseButton,
			MouseMoved,
			MouseScrolled
		}; // enum class EventType
	} // namespace Events
} // namespace Grindstone
