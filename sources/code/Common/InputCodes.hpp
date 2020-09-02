#pragma once

#include <Common/Rendering/RenderPathBase.hpp>

namespace Grindstone {
	namespace Input {
		enum class MouseAction {
			Left = 0,
			Middle,
			Right,

			Mouse4,
			Mouse5,

			WheelUp,
			WheelDown,
			WheelLeft,
			WheelRight,

			// X and Y Position
			XCoord,
			YCoord,
			Last
		};

		enum class KeyAction {
			ArrowLeft = 0,
			ArrowRight,
			ArrowUp,
			ArrowDown,

			Numpad0,
			Numpad1,
			Numpad2,
			Numpad3,
			Numpad4,
			Numpad5,
			Numpad6,
			Numpad7,
			Numpad8,
			Numpad9,

			NumpadLock,
			NumpadDivide,
			NumpadMultiply,
			NumpadSubtract,
			NumpadAdd,
			NumpadEnter,
			NumpadDot,

			F0,
			F1,
			F2,
			F3,
			F4,
			F5,
			F6,
			F7,
			F8,
			F9,
			F10,
			F11,
			F12,
			F13,
			F14,
			F15,
			F16,
			F17,
			F18,
			F19,
			F20,
			F21,
			F22,
			F23,
			F24,
			F25,

			Num0,
			Num1,
			Num2,
			Num3,
			Num4,
			Num5,
			Num6,
			Num7,
			Num8,
			Num9,
			DASH,
			ADD,

			A,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,

			Escape,
			LeftShift,
			Shift,
			LeftControl,
			Control,
			LeftAlt,
			Alt,
			CapsLock,
			Tab,

			Insert,
			Home,
			PageUp,
			PageDown,
			End,
			Delete,
			Pause,
			ScrollLock,

			Comma,
			Period,
			ForwardSlash,
			Semicolon,
			Apostrophe,
			BackSlash,
			LeftBracket,
			RightBracket,

			Enter,
			Backspace,
			Tilde,

			Window,

			Space,
			Last,
			None = Last
		};

		enum class WindowEvent {
			Close = 0,
			ForceClose,
			ResizeX,
			ResizeY,
			MoveX,
			MoveY,
			Focus,
			Minimize,
			Last
		};

		enum class DeviceType{
			Window = 0,
			Keyboard,
			Mouse
		};

		enum class ButtonStatus {
			Pressed = 0,
			Released
		};

	} // namespace Input
} // namespace Grindstone
