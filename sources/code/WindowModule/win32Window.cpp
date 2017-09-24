#include "../GraphicsCommon/GraphicsWrapper.h"

#if defined(_WIN32) && !defined(GLFW_WINDOW)
#include "../Engine/Core/Input.h"

int TranslateKey(int key);

LRESULT CALLBACK GraphicsWrapper::sWndProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	GraphicsWrapper *pThis;

	if (uMsg == WM_NCCREATE) {
		// Recover the "this" pointer which was passed as a parameter to CreateWindow(Ex).
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<GraphicsWrapper*>(lpcs->lpCreateParams);
		// Put the value in a safe place for future use
		SetWindowLongPtr(hwnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(pThis));
	}
	else {
		// Recover the "this" pointer from where our WM_NCCREATE handler stashed it.
		pThis = reinterpret_cast<GraphicsWrapper*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (pThis) {
		return pThis->WndProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK GraphicsWrapper::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!input)
		return DefWindowProc(hwnd, msg, wParam, lParam);
	
	switch (msg) {
	case WM_SIZE:
		input->ResizeEvent(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		input->SetMousePosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_SETFOCUS:
		input->SetFocused(true);
		break;
	case WM_KILLFOCUS:
		input->SetFocused(false);
		break;
	case WM_MOUSEHWHEEL:
		if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
			input->SetMouseButton(MOUSE_WHEEL_LEFT, true);
		else
			input->SetMouseButton(MOUSE_WHEEL_RIGHT, true);
		break;
	case WM_MOUSEWHEEL:
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			input->SetMouseButton(MOUSE_WHEEL_UP, true);
		else
			input->SetMouseButton(MOUSE_WHEEL_DOWN, true);
		break;
	case WM_LBUTTONDOWN:
		input->SetMouseButton(MOUSE_LEFT, true);
		break;
	case WM_LBUTTONUP:
		input->SetMouseButton(MOUSE_LEFT, false);
		break;
	case WM_MBUTTONDOWN:
		input->SetMouseButton(MOUSE_MIDDLE, true);
		break;
	case WM_MBUTTONUP:
		input->SetMouseButton(MOUSE_MIDDLE, false);
		break;
	case WM_RBUTTONDOWN:
		input->SetMouseButton(MOUSE_RIGHT, true);
		break;
	case WM_RBUTTONUP:
		input->SetMouseButton(MOUSE_RIGHT, false);
		break;
	case WM_XBUTTONDOWN:
		input->SetMouseButton(MOUSE_MOUSE4, true); //MOUSE_MOUSE5 as well
		break;
	case WM_XBUTTONUP:
		input->SetMouseButton(MOUSE_MOUSE4, false);
		break;
	case WM_KEYDOWN:
		// Repeat Count
		if ((HIWORD(lParam) & KF_REPEAT) == 0) {
			input->SetKey(TranslateKey(int(wParam)), true);
		}
		break;
	case WM_KEYUP:
		input->SetKey(TranslateKey(int(wParam)), false);
		break;
	case WM_CLOSE:
		input->Quit();
		break;
	case WM_DESTROY:
		input->ForceQuit();
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void GraphicsWrapper::SetCursorShown(bool shown) {
	ShowCursor(shown);
}

bool GraphicsWrapper::InitializeWin32Window() {
	int fullscreen = 2;

	WNDCLASSEX wc;

	const char *className = "GameWindow";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = GraphicsWrapper::sWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	DWORD style, styleEx;
	switch (fullscreen) {
	case 0: // Windowed
		style = WS_OVERLAPPEDWINDOW;
		styleEx = WS_EX_CLIENTEDGE;
		break;
	case 1: // Borderless Windowed
		style = WS_OVERLAPPEDWINDOW;
		styleEx = WS_EX_CLIENTEDGE | WS_EX_TOPMOST;
		break;
	case 2: // Fullscreen
		style = WS_OVERLAPPEDWINDOW;
		styleEx = WS_EX_CLIENTEDGE;
		break;
	}

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRectEx(&rect, style, FALSE, styleEx);

	window_handle = CreateWindowEx(
		styleEx,
		className,
		title,
		style,
		0,
		0,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		this);

	if (window_handle == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	ShowWindow(window_handle, SW_SHOW);
	UpdateWindow(window_handle);

	return true;
}

void GraphicsWrapper::HandleEvents() {
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int TranslateKey(int key) {
	//static int keyCodes[] = { KEY_BACKSPACE, KEY_TAB, KEY_NONE, KEY_NONE, KEY_ENTER, KEY_NONE, KEY_NONE, KEY_SHIFT, KEY_CONTROL, KEY_ALT, KEY_PAUSE, KEY_CAPSLOCK, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_ESCAPE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_SPACE, KEY_PG_UP, KEY_PG_DOWN, KEY_END, KEY_HOME, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_SELECT, KEY_PRINT, KEY_EXECUTE, KEY_PRINTSCR, KEY_DELETE, KEY_HELP, KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_NONE, KEY_NONE, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_LWINDOW, KEY_WINDOW, KEY_APPS, KEY_NONE, KEY_NONE, KEY_NUMPAD_0, KEY_NUMPAD_1, KEY_NUMPAD_2, KEY_NUMPAD_3, KEY_NUMPAD_4, KEY_NUMPAD_5, KEY_NUMPAD_6, KEY_NUMPAD_7, KEY_NUMPAD_8, KEY_NUMPAD_9, KEY_NUMPAD_MULTIPLY, KEY_NUMPAD_ADD, KEY_NUMPAD_ENTER, KEY_NUMPAD_SUBTRACT, KEY_NUMPAD_DOT, KEY_NUMPAD_DIVIDE, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_NONE, KEY_NUMPAD_NUMLOCK, KEY_SCROLL_LOCK, KEY_NONE, KEY_LSHIFT, KEY_LCONTROL, KEY_CONTROL, KEY_LALT, KEY_ALT, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE };
	switch (key) {
	case 0x41: return KEY_A;
	case 0x42: return KEY_B;
	case 0x43: return KEY_C;
	case 0x44: return KEY_D;
	case 0x45: return KEY_E;
	case 0x46: return KEY_F;
	case 0x47: return KEY_G;
	case 0x48: return KEY_H;
	case 0x49: return KEY_I;
	case 0x4A: return KEY_J;
	case 0x4B: return KEY_K;
	case 0x4C: return KEY_L;
	case 0x4D: return KEY_M;
	case 0x4E: return KEY_N;
	case 0x4F: return KEY_O;
	case 0x50: return KEY_P;
	case 0x51: return KEY_Q;
	case 0x52: return KEY_R;
	case 0x53: return KEY_S;
	case 0x54: return KEY_T;
	case 0x55: return KEY_U;
	case 0x56: return KEY_V;
	case 0x57: return KEY_W;
	case 0x58: return KEY_X;
	case 0x59: return KEY_Y;
	case 0x5A: return KEY_Z;

	case VK_ESCAPE: return KEY_ESCAPE;
	case VK_TAB: return KEY_TAB;
	case VK_SPACE: return KEY_SPACE;
	case VK_LCONTROL: return KEY_LCONTROL;
	case VK_CONTROL: return KEY_CONTROL;
	case VK_LSHIFT: return KEY_LSHIFT;
	case VK_SHIFT: return KEY_SHIFT;
	case VK_LMENU: return KEY_LALT;
	case VK_MENU: return KEY_ALT;

	case VK_LEFT:	return KEY_LEFT;
	case VK_RIGHT:	return KEY_RIGHT;
	case VK_UP:	return KEY_UP;
	case VK_DOWN:	return KEY_DOWN;

	case VK_NUMPAD0: return KEY_NUMPAD_0;
	case VK_NUMPAD1: return KEY_NUMPAD_1;
	case VK_NUMPAD2: return KEY_NUMPAD_2;
	case VK_NUMPAD3: return KEY_NUMPAD_3;
	case VK_NUMPAD4: return KEY_NUMPAD_4;
	case VK_NUMPAD5: return KEY_NUMPAD_5;
	case VK_NUMPAD6: return KEY_NUMPAD_6;
	case VK_NUMPAD7: return KEY_NUMPAD_7;
	case VK_NUMPAD8: return KEY_NUMPAD_8;
	case VK_NUMPAD9: return KEY_NUMPAD_9;

	case VK_NUMLOCK:	return KEY_NUMPAD_NUMLOCK;
	case VK_DIVIDE: 	return KEY_NUMPAD_DIVIDE;
	case VK_MULTIPLY: 	return KEY_NUMPAD_MULTIPLY;
	case VK_SUBTRACT: 	return KEY_NUMPAD_SUBTRACT;
	case VK_ADD: 		return KEY_NUMPAD_ADD;
	case VK_SEPARATOR: 	return KEY_NUMPAD_ENTER;
	case VK_DECIMAL: 	return KEY_NUMPAD_DOT;

	case VK_F1: return KEY_F1;
	case VK_F2: return KEY_F2;
	case VK_F3: return KEY_F3;
	case VK_F4: return KEY_F4;
	case VK_F5: return KEY_F5;
	case VK_F6: return KEY_F6;
	case VK_F7: return KEY_F7;
	case VK_F8: return KEY_F8;
	case VK_F9: return KEY_F9;
	case VK_F10:return KEY_F10;
	case VK_F11:return KEY_F11;
	case VK_F12:return KEY_F12;
	case VK_F13:return KEY_F13;
	case VK_F14:return KEY_F14;
	case VK_F15:return KEY_F15;
	case VK_F16:return KEY_F16;
	case VK_F17:return KEY_F17;
	case VK_F18:return KEY_F18;
	case VK_F19:return KEY_F19;
	case VK_F20:return KEY_F20;
	case VK_F21:return KEY_F21;
	case VK_F22:return KEY_F22;
	case VK_F23:return KEY_F23;
	case VK_F24:return KEY_F24;

	case 0x30:	return KEY_0;
	case 0x31:	return KEY_1;
	case 0x32:	return KEY_2;
	case 0x33:	return KEY_3;
	case 0x34:	return KEY_4;
	case 0x35:	return KEY_5;
	case 0x36:	return KEY_6;
	case 0x37:	return KEY_7;
	case 0x38:	return KEY_8;
	case 0x39:	return KEY_9;
	case VK_OEM_MINUS:	return KEY_DASH;
	case VK_OEM_PLUS:	return KEY_ADD;

	case VK_INSERT:		return KEY_INSERT;
	case VK_HOME:		return KEY_HOME;
	case VK_NEXT:	return KEY_PG_UP;
	case VK_PRIOR:	return KEY_PG_DOWN;
	case VK_END:		return KEY_END;
	case VK_DELETE:	return KEY_DELETE;
	case VK_PAUSE:		return KEY_PAUSE;
	case VK_CAPITAL:	return KEY_CAPSLOCK;
	case VK_SCROLL:	return KEY_SCROLL_LOCK;

	case VK_OEM_COMMA:		return KEY_COMMA;
	case VK_OEM_PERIOD:		return KEY_PERIOD;
	case VK_OEM_2:			return KEY_FORWARD_SLASH;
	case VK_OEM_5:			return KEY_BACK_SLASH;
	case VK_OEM_1:			return KEY_SEMICOLON;
	case VK_OEM_7:			return KEY_APOSTROPHE;
	case VK_OEM_4:			return KEY_LBRACKET;
	case VK_OEM_6:			return KEY_RBRACKET;

	case VK_RETURN:			return KEY_ENTER;
	case VK_BACK:			return KEY_BACKSPACE;
	case VK_OEM_3:			return KEY_TILDE;
	case VK_LWIN:			return KEY_WINDOW;
	case VK_RWIN:			return KEY_WINDOW;
	default:				return -1;
	}
}

void GraphicsWrapper::ResetCursor() {
	SetCursor(width / 2, height / 2);
}

void GraphicsWrapper::SetCursor(int x, int y) {
	POINT pt;
	pt.x = (LONG)x;
	pt.y = (LONG)y;
	ClientToScreen(window_handle, &pt);
	SetCursorPos(pt.x, pt.y);
}

void GraphicsWrapper::GetCursor(int &x, int &y) {
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(window_handle, &p);
	x = p.x;
	y = p.y;
}

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return std::string(); //No error message has been recorded

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

#endif