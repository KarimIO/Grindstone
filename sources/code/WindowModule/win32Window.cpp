#ifdef _WIN32
#include "Window.h"
#include "Input.h"

int TranslateKey(int key);

LRESULT CALLBACK GameWindow::sWndProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	GameWindow *pThis;

	if (uMsg == WM_NCCREATE) {
		// Recover the "this" pointer which was passed as a parameter to CreateWindow(Ex).
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<GameWindow*>(lpcs->lpCreateParams);
		// Put the value in a safe place for future use
		SetWindowLongPtr(hwnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(pThis));
	}
	else {
		// Recover the "this" pointer from where our WM_NCCREATE handler stashed it.
		pThis = reinterpret_cast<GameWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (pThis) {
		return pThis->WndProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK GameWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (!input)
		return DefWindowProc(hwnd, msg, wParam, lParam);
	
	switch (msg) {
	case WM_SIZE:
		input->ResizeEvent(LOWORD(lParam), HIWORD(lParam));
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
		input->SetKey(TranslateKey(int(wParam)), true);
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

GameWindow::~GameWindow() {
	std::cout << "Shut Down" << std::endl;
}

bool GameWindow::Initialize(const char *title, int resolutionX, int resolutionY) {
	resX = resolutionX;
	resY = resolutionY;

	WNDCLASSEX wc;

	const char *className = "GameWindow";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = GameWindow::sWndProc;
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

	window_handle = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		className,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, resX, resY,
		NULL, NULL, GetModuleHandle(NULL), this);

	if (window_handle == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return false;
	}

	ShowWindow(window_handle, SW_SHOW);
	UpdateWindow(window_handle);

	return true;
}

void GameWindow::HandleEvents() {
	MSG msg;
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int TranslateKey(int key) {
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
	case VK_OEM_2:		return KEY_FORWARD_SLASH;
	case VK_OEM_5:	return KEY_BACK_SLASH;
	case VK_OEM_1:	return KEY_SEMICOLON;
	case VK_OEM_7:	return KEY_APOSTROPHE;
	case VK_OEM_4:	return KEY_LBRACKET;
	case VK_OEM_6:	return KEY_RBRACKET;

	case VK_RETURN:		return KEY_ENTER;
	case VK_BACK:		return KEY_BACKSPACE;
	case VK_OEM_3:		return KEY_TILDE;
	default: return -1;
	}
}

void GameWindow::ResetCursor() {
	SetCursor(resX / 2, resY / 2);
}

void GameWindow::SetCursor(int x, int y) {
	POINT pt;
	pt.x = (LONG)x;
	pt.y = (LONG)y;
	ClientToScreen(window_handle, &pt);
	SetCursorPos(pt.x, pt.y);
}

void GameWindow::GetCursor(int &x, int &y) {
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(window_handle, &p);
	x = p.x;
	y = p.y;
}

void GameWindow::Shutdown() {
	delete this;
}

#endif