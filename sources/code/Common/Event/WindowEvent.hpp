#pragma once

#include "BaseEvent.hpp"
#include "KeyPressCode.hpp"

namespace Grindstone {
	namespace Events {
		struct WindowCloseEvent : public BaseEvent {
			SETUP_EVENT(WindowClose)
		}; // struct WindowClose

		struct WindowResizeEvent : public BaseEvent {
			WindowResizeEvent(int width, int height)
				: width(width), height(height) {}
			int width;
			int height;

			SETUP_EVENT(WindowResize)
		}; // struct WindowResize

		struct WindowStartFocusEvent : public BaseEvent {
			SETUP_EVENT(WindowStartFocus)
		}; // struct WindowStartFocus

		struct WindowKillFocusEvent : public BaseEvent {
			SETUP_EVENT(WindowKillFocus)
		}; // struct WindowKillFocus

		struct WindowMovedEvent : public BaseEvent {
			WindowMovedEvent(int windowPositionX, int windowPositionY)
				: windowPositionX(windowPositionX), windowPositionY(windowPositionY) {}
			int windowPositionX;
			int windowPositionY;

			SETUP_EVENT(WindowMoved)
		}; // struct WindowMoved
	} // namespace Events
} // namespace Grindstone
