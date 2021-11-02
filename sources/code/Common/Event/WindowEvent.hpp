#pragma once

#include "BaseEvent.hpp"
#include "KeyPressCode.hpp"

namespace Grindstone {
	class Window;

	namespace Events {
		struct WindowTryQuitEvent : public BaseEvent {
			WindowTryQuitEvent(Grindstone::Window* window)
				: window(window) {}
			Grindstone::Window* window;
			SETUP_EVENT(WindowTryQuit)
		}; // struct WindowTryQuitEvent

		struct WindowForceQuitEvent : public BaseEvent {
			WindowForceQuitEvent(Grindstone::Window* window)
				: window(window) {}
			Grindstone::Window* window;
			SETUP_EVENT(WindowForceQuit)
		}; // struct WindowForceQuitEvent

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
