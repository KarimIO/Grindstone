#ifndef _INPUT_INTERFACE_H
#define _INPUT_INTERFACE_H
// This file is used to get the inputs from the windows module. Is there a better way to do this??


enum {
	MOUSE_LEFT = 0,
	MOUSE_MIDDLE,
	MOUSE_RIGHT,

	MOUSE_MOUSE4,
	MOUSE_MOUSE5,

	MOUSE_WHEEL_UP,
	MOUSE_WHEEL_DOWN,
	MOUSE_WHEEL_LEFT,
	MOUSE_WHEEL_RIGHT,

	// X and Y Position
	MOUSE_XCOORD,
	MOUSE_YCOORD,
	MOUSE_LAST
};

enum {
	KEY_LEFT = 0,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,

	KEY_NUMPAD_0,
	KEY_NUMPAD_1,
	KEY_NUMPAD_2,
	KEY_NUMPAD_3,
	KEY_NUMPAD_4,
	KEY_NUMPAD_5,
	KEY_NUMPAD_6,
	KEY_NUMPAD_7,
	KEY_NUMPAD_8,
	KEY_NUMPAD_9,

	KEY_NUMPAD_NUMLOCK,
	KEY_NUMPAD_DIVIDE,
	KEY_NUMPAD_MULTIPLY,
	KEY_NUMPAD_SUBTRACT,
	KEY_NUMPAD_ADD,
	KEY_NUMPAD_ENTER,
	KEY_NUMPAD_DOT,

	KEY_F0,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12,
	KEY_F13,
	KEY_F14,
	KEY_F15,
	KEY_F16,
	KEY_F17,
	KEY_F18,
	KEY_F19,
	KEY_F20,
	KEY_F21,
	KEY_F22,
	KEY_F23,
	KEY_F24,
	KEY_F25,

	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_DASH,
	KEY_ADD,

	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,

	KEY_ESCAPE,
	KEY_LSHIFT,
	KEY_SHIFT,
	KEY_LCONTROL,
	KEY_CONTROL,
	KEY_LALT,
	KEY_ALT,
	KEY_CAPSLOCK,
	KEY_TAB,

	KEY_INSERT,
	KEY_HOME,
	KEY_PG_UP,
	KEY_PG_DOWN,
	KEY_END,
	KEY_DELETE,
	KEY_PAUSE,
	KEY_SCROLL_LOCK,

	KEY_COMMA,
	KEY_PERIOD,
	KEY_FORWARD_SLASH,
	KEY_SEMICOLON,
	KEY_APOSTROPHE,
	KEY_BACK_SLASH,
	KEY_LBRACKET,
	KEY_RBRACKET,

	KEY_ENTER,
	KEY_BACKSPACE,
	KEY_TILDE,

	KEY_WINDOW,

	KEY_SPACE,
	KEY_LAST,
	KEY_NONE = KEY_LAST
};

enum {
	WINDOW_CLOSE = 0,
	WINDOW_FORCECLOSE,
	WINDOW_RESIZEX,
	WINDOW_RESIZEY,
	WINDOW_MOVEX,
	WINDOW_MOVEY,
	WINDOW_FOCUS,
	WINDOW_MINIMIZE,
	WINDOW_LAST
};

enum {
	DEVICE_WINDOW = 0,
	DEVICE_KEYBOARD,
	DEVICE_MOUSE
};

enum KEY_STATUS {
	KEY_PRESSED = 0,
	KEY_RELEASED
};

// This exists only to pass a smaller pointer and have fewer requirements to the other DLLs
class InputInterface {
public:
    virtual void ResizeEvent(int, int) = 0;
    virtual void SetMouseButton(int, bool) = 0;
	virtual void SetMousePosition(int, int) = 0;
	virtual void SetFocused(bool) = 0;
	virtual bool IsFocused() = 0;
    virtual void SetKey(int, bool) = 0;
    virtual void Quit() = 0;
    virtual void ForceQuit() = 0;
};

#endif