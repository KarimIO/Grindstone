//#include "../Engine/Core/InputInterface.hpp"
#include "Win32Window.hpp"
#include <EngineCore/EngineCore.hpp>
#include <Common/Input.hpp>
#include <GL/gl3w.h>
#include <GL/wglext.h>
#include <Windowsx.h>

#include <iostream>
#include <assert.h>
#include <vector>

#include <tchar.h>

using namespace Grindstone;

Events::KeyPressCode TranslateKey(int key);

LRESULT CALLBACK Win32Window::sWndProc(
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	Win32Window* pThis;

	if (uMsg == WM_NCCREATE) {
		// Recover the "this" pointer which was passed as a parameter to CreateWindow(Ex).
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<Win32Window*>(lpcs->lpCreateParams);
		// Put the value in a safe place for future use
		SetWindowLongPtr(hwnd, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(pThis));
	}
	else {
		// Recover the "this" pointer from where our WM_NCCREATE handler stashed it.
		pThis = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	}

	if (pThis) {
		return pThis->WndProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK Win32Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Input::Interface* input = nullptr;
	if (engineCore) {
		input = engineCore->getInputManager();
		if (input == nullptr) {
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	else {
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	switch (msg) {
	case WM_SIZE:
		input->ResizeEvent(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		input->SetMousePosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_SETFOCUS:
		input->SetIsFocused(true);
		break;
	case WM_KILLFOCUS:
		input->SetIsFocused(false);
		break;
	case WM_MOUSEHWHEEL:
		input->MouseScroll(GET_WHEEL_DELTA_WPARAM(wParam), 0);
		break;
	case WM_MOUSEWHEEL:
		input->MouseScroll(0, GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	case WM_LBUTTONDOWN:
		input->SetMouseButton(Events::MouseButtonCode::Left, true);
		break;
	case WM_LBUTTONUP:
		input->SetMouseButton(Events::MouseButtonCode::Left, false);
		break;
	case WM_MBUTTONDOWN:
		input->SetMouseButton(Events::MouseButtonCode::Middle, true);
		break;
	case WM_MBUTTONUP:
		input->SetMouseButton(Events::MouseButtonCode::Middle, false);
		break;
	case WM_RBUTTONDOWN:
		input->SetMouseButton(Events::MouseButtonCode::Right, true);
		break;
	case WM_RBUTTONUP:
		input->SetMouseButton(Events::MouseButtonCode::Right, false);
		break;
	case WM_XBUTTONDOWN:
		input->SetMouseButton(Events::MouseButtonCode::Mouse4, true); //MOUSE_MOUSE5 as well
		break;
	case WM_XBUTTONUP:
		input->SetMouseButton(Events::MouseButtonCode::Mouse4, false);
		break;
	case WM_SYSCHAR:
	case WM_CHAR:
		if (wParam > 0 && wParam < 0x10000) {
			input->AddCharacterTyped((unsigned short)wParam);
		}
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		// Repeat Count
		if ((HIWORD(lParam) & KF_REPEAT) == 0) {
			input->SetKeyPressed(TranslateKey(int(wParam)), true);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		input->SetKeyPressed(TranslateKey(int(wParam)), false);
		break;
	case WM_CREATE:
		input->SetIsFocused(true);
		break;
	case WM_CLOSE:
		if (MessageBox(NULL, _T("Are you sure you want to cancel?"), _T("Error!"), MB_ICONEXCLAMATION | MB_YESNO)) {
			shouldClose = true;
			DestroyWindow(hwnd);
		}
		break;
	case WM_DESTROY:
		shouldClose = true;
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Win32Window::Initialize(CreateInfo& createInfo) {
	auto hInstance = GetModuleHandle(NULL);

	shouldClose = false;
	width = createInfo.width;
	height = createInfo.height;
	fullscreenMode = createInfo.fullscreen;
	engineCore = createInfo.engineCore;
	auto display = createInfo.display;

	extendedStyle = 0;

	switch (fullscreenMode) {
	case FullscreenMode::Borderless: {
		windowSize = RECT{
			(LONG)display.x,
			(LONG)display.y,
			(LONG)(display.x + display.width),
			(LONG)(display.y + display.height)
		};
		style = WS_POPUP;
		break;
	}
	case FullscreenMode::Fullscreen: {
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsHeight = (unsigned long)height;
		dmScreenSettings.dmPelsWidth = (unsigned long)width;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		dmScreenSettings.dmPosition.x = display.x;
		dmScreenSettings.dmPosition.y = display.y;

		windowSize.left = 0;
		windowSize.top = 0;
		windowSize.right = (unsigned long)width;
		windowSize.bottom = (unsigned long)height;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
		style = WS_POPUP;
		break;
	}
	case FullscreenMode::Windowed:
		windowSize.left = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		windowSize.top = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
		windowSize.right = windowSize.left + width;
		windowSize.bottom = windowSize.top + height;
		style = WS_OVERLAPPEDWINDOW;
		break;
	}

	AdjustWindowRect(&windowSize, style, FALSE);

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = Win32Window::sWndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = createInfo.title;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, _T("Error registering class"),
			_T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	windowHandle = CreateWindowEx(extendedStyle,
		createInfo.title,
		createInfo.title,
		style,
		windowSize.left,
		windowSize.top,
		windowSize.right - windowSize.left,
		windowSize.bottom - windowSize.top,
		NULL,
		NULL,
		hInstance,
		this
	);

	if (!windowHandle) {
		MessageBox(NULL, _T("Error creating window"),
			_T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

void Win32Window::Show() {
	ShowWindow(windowHandle, SW_SHOW);
	UpdateWindow(windowHandle);
}

bool Win32Window::ShouldClose() {
	return shouldClose;
}

bool Win32Window::HandleEvents() {
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		// Translate the message and dispatch it to WindowProc()
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
	}

	return true;
}

void Win32Window::SetFullscreen(FullscreenMode mode) {
	fullscreenMode = mode;

	DWORD style;

	switch (fullscreenMode) {
	case FullscreenMode::Windowed: {
		windowSize.left = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
		windowSize.top = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
		windowSize.right = windowSize.left + width;
		windowSize.bottom = windowSize.top + height;
		style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;

		SetWindowLongPtr(windowHandle, GWL_STYLE, style);
		::SetWindowPos(windowHandle, NULL, 0, 0, width, height, SWP_FRAMECHANGED);
		break;
	}
	case FullscreenMode::Fullscreen:
	{
		// TODO: Add devmode stuff
		windowSize.left = 0;
		windowSize.top = 0;
		windowSize.right = width;
		windowSize.bottom = height;
		style = WS_VISIBLE | WS_POPUP;

		SetWindowLongPtr(windowHandle, GWL_STYLE, WS_VISIBLE | WS_POPUP);
		::SetWindowPos(windowHandle, HWND_TOP, 0, 0, windowSize.right, windowSize.bottom, SWP_FRAMECHANGED);
		break;
	}
	default:
	case FullscreenMode::Borderless:
	{
		windowSize.left = 0;
		windowSize.top = 0;
		windowSize.right = GetSystemMetrics(SM_CXSCREEN);
		windowSize.bottom = GetSystemMetrics(SM_CYSCREEN);
		style = WS_VISIBLE | WS_POPUP;

		SetWindowLongPtr(windowHandle, GWL_STYLE, WS_VISIBLE | WS_POPUP);
		::SetWindowPos(windowHandle, HWND_TOP, 0, 0, windowSize.right, windowSize.bottom, SWP_FRAMECHANGED);
		break;
	}
	}

	AdjustWindowRect(&windowSize, style, FALSE);
}


void Win32Window::GetWindowRect(unsigned int &left, unsigned int &top, unsigned int &right, unsigned int &bottom) {
	RECT rect;
	::GetWindowRect(windowHandle, &rect);
	left = rect.left;
	top = rect.top;
	right = rect.right;
	bottom = rect.bottom;
}

void Win32Window::GetWindowSize(unsigned int &width, unsigned int &height) {
	RECT rect;
	::GetClientRect(windowHandle, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

void Win32Window::SetWindowSize(unsigned int width, unsigned int height) {
	RECT rect = { 0, 0, (LONG)width, (LONG)height };
	::AdjustWindowRectEx(&rect, style, FALSE, extendedStyle); // Client to Screen
	::SetWindowPos(windowHandle, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

}

void Win32Window::SetMousePos(unsigned int x, unsigned int y) {
	POINT pt;
	pt.x = (LONG)x;
	pt.y = (LONG)y;
	ClientToScreen(windowHandle, &pt);
	SetCursorPos(pt.x, pt.y);
}

void Win32Window::GetMousePos(unsigned int& x, unsigned int& y) {
	POINT point;
	GetCursorPos(&point);

	x = point.x;
	y = point.y;
}

void Win32Window::SetWindowPos(unsigned int x, unsigned int y) {
	RECT rect = { (LONG)x, (LONG)y, (LONG)x, (LONG)y };
	::AdjustWindowRectEx(&rect, style, FALSE, extendedStyle);
	::SetWindowPos(windowHandle, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void Win32Window::GetWindowPos(unsigned int& width, unsigned int& height) {
	POINT pos = { 0, 0 };
	::ClientToScreen(windowHandle, &pos);
	width = pos.x;
	height = pos.y;
}

void Win32Window::Close() {
	DestroyWindow(windowHandle);
	// UnregisterClassA("GameWindow", GetModuleHandle(NULL));
}

HWND Win32Window::GetHandle() {
	return windowHandle;
}

void Win32Window::SetWindowFocus() {
	::BringWindowToTop(windowHandle);
	::SetForegroundWindow(windowHandle);
	::SetFocus(windowHandle);
}

bool Win32Window::GetWindowFocus() {
	return ::GetForegroundWindow() == windowHandle;
}

bool Win32Window::GetWindowMinimized() {
	return ::IsIconic(windowHandle) != 0;
}

void Win32Window::SetWindowTitle(const char* title) {
	// ::SetWindowTextA() doesn't properly handle UTF-8 so we explicitely convert our string.
	int n = ::MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
	std::vector<wchar_t> title_w;
	title_w.resize(n);
	::MultiByteToWideChar(CP_UTF8, 0, title, -1, title_w.data(), n);
	::SetWindowTextW(windowHandle, title_w.data());
}

void Win32Window::SetWindowAlpha(float alpha) {
	//IM_ASSERT(alpha >= 0.0f && alpha <= 1.0f);
	if (alpha < 1.0f) {
		DWORD style = ::GetWindowLongW(windowHandle, GWL_EXSTYLE) | WS_EX_LAYERED;
		::SetWindowLongW(windowHandle, GWL_EXSTYLE, style);
		::SetLayeredWindowAttributes(windowHandle, 0, (BYTE)(255 * alpha), LWA_ALPHA);
	}
	else {
		DWORD style = ::GetWindowLongW(windowHandle, GWL_EXSTYLE) & ~WS_EX_LAYERED;
		::SetWindowLongW(windowHandle, GWL_EXSTYLE, style);
	}
}

float Win32Window::GetWindowDpiScale() {
	/*HMONITOR monitor = ::MonitorFromWindow(windowHandle, MONITOR_DEFAULTTONEAREST);
	UINT xdpi = 96, ydpi = 96;
	if (::IsWindows8Point1OrGreater())
	{
		static HINSTANCE shcore_dll = ::LoadLibraryA("shcore.dll"); // Reference counted per-process
		if (PFN_GetDpiForMonitor GetDpiForMonitorFn = (PFN_GetDpiForMonitor)::GetProcAddress(shcore_dll, "GetDpiForMonitor"))
			GetDpiForMonitorFn((HMONITOR)monitor, MDT_EFFECTIVE_DPI, &xdpi, &ydpi);
	}
	else
	{
		const HDC dc = ::GetDC(NULL);
		xdpi = ::GetDeviceCaps(dc, LOGPIXELSX);
		ydpi = ::GetDeviceCaps(dc, LOGPIXELSY);
		::ReleaseDC(NULL, dc);
	}
	IM_ASSERT(xdpi == ydpi); // Please contact me if you hit this assert!
	return xdpi / 96.0f;*/
	return 1.0f;
}

Events::KeyPressCode TranslateKey(int key) {
	//static int keyCodes[] = { KEY_BACKSPACE, KEY_TAB, KEY_NONE, KEY_NONE, KEY_ENTER, KEY_NONE, KEY_NONE, KEY_SHIFT, KEY_CONTROL, KEY_ALT, KEY_PAUSE, KEY_CAPSLOCK, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_ESCAPE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_SPACE, KEY_PG_UP, KEY_PG_DOWN, KEY_END, KEY_HOME, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_SELECT, KEY_PRINT, KEY_EXECUTE, KEY_PRINTSCR, KEY_DELETE, KEY_HELP, KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_NONE, KEY_NONE, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_LWINDOW, KEY_WINDOW, KEY_APPS, KEY_NONE, KEY_NONE, KEY_NUMPAD_0, KEY_NUMPAD_1, KEY_NUMPAD_2, KEY_NUMPAD_3, KEY_NUMPAD_4, KEY_NUMPAD_5, KEY_NUMPAD_6, KEY_NUMPAD_7, KEY_NUMPAD_8, KEY_NUMPAD_9, KEY_NUMPAD_MULTIPLY, KEY_NUMPAD_ADD, KEY_NUMPAD_ENTER, KEY_NUMPAD_SUBTRACT, KEY_NUMPAD_DOT, KEY_NUMPAD_DIVIDE, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_NONE, KEY_NUMPAD_NUMLOCK, KEY_SCROLL_LOCK, KEY_NONE, KEY_LSHIFT, KEY_LCONTROL, KEY_CONTROL, KEY_LALT, KEY_ALT, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE };
	switch (key) {
	case 0x41: return Events::KeyPressCode::A;
	case 0x42: return Events::KeyPressCode::B;
	case 0x43: return Events::KeyPressCode::C;
	case 0x44: return Events::KeyPressCode::D;
	case 0x45: return Events::KeyPressCode::E;
	case 0x46: return Events::KeyPressCode::F;
	case 0x47: return Events::KeyPressCode::G;
	case 0x48: return Events::KeyPressCode::H;
	case 0x49: return Events::KeyPressCode::I;
	case 0x4A: return Events::KeyPressCode::J;
	case 0x4B: return Events::KeyPressCode::K;
	case 0x4C: return Events::KeyPressCode::L;
	case 0x4D: return Events::KeyPressCode::M;
	case 0x4E: return Events::KeyPressCode::N;
	case 0x4F: return Events::KeyPressCode::O;
	case 0x50: return Events::KeyPressCode::P;
	case 0x51: return Events::KeyPressCode::Q;
	case 0x52: return Events::KeyPressCode::R;
	case 0x53: return Events::KeyPressCode::S;
	case 0x54: return Events::KeyPressCode::T;
	case 0x55: return Events::KeyPressCode::U;
	case 0x56: return Events::KeyPressCode::V;
	case 0x57: return Events::KeyPressCode::W;
	case 0x58: return Events::KeyPressCode::X;
	case 0x59: return Events::KeyPressCode::Y;
	case 0x5A: return Events::KeyPressCode::Z;

	case VK_ESCAPE: return Events::KeyPressCode::Escape;
	case VK_TAB: return Events::KeyPressCode::Tab;
	case VK_SPACE: return Events::KeyPressCode::Space;
	case VK_LCONTROL: return Events::KeyPressCode::LeftControl;
	case VK_CONTROL: return Events::KeyPressCode::Control;
	case VK_LSHIFT: return Events::KeyPressCode::LeftShift;
	case VK_SHIFT: return Events::KeyPressCode::Shift;
	case VK_LMENU: return Events::KeyPressCode::LeftAlt;
	case VK_MENU: return Events::KeyPressCode::Alt;

	case VK_LEFT:	return Events::KeyPressCode::ArrowLeft;
	case VK_RIGHT:	return Events::KeyPressCode::ArrowRight;
	case VK_UP:	return Events::KeyPressCode::ArrowUp;
	case VK_DOWN:	return Events::KeyPressCode::ArrowDown;

	case VK_NUMPAD0: return Events::KeyPressCode::Numpad0;
	case VK_NUMPAD1: return Events::KeyPressCode::Numpad1;
	case VK_NUMPAD2: return Events::KeyPressCode::Numpad2;
	case VK_NUMPAD3: return Events::KeyPressCode::Numpad3;
	case VK_NUMPAD4: return Events::KeyPressCode::Numpad4;
	case VK_NUMPAD5: return Events::KeyPressCode::Numpad5;
	case VK_NUMPAD6: return Events::KeyPressCode::Numpad6;
	case VK_NUMPAD7: return Events::KeyPressCode::Numpad7;
	case VK_NUMPAD8: return Events::KeyPressCode::Numpad8;
	case VK_NUMPAD9: return Events::KeyPressCode::Numpad9;

	case VK_NUMLOCK:	return Events::KeyPressCode::NumpadLock;
	case VK_DIVIDE: 	return Events::KeyPressCode::NumpadDivide;
	case VK_MULTIPLY: 	return Events::KeyPressCode::NumpadMultiply;
	case VK_SUBTRACT: 	return Events::KeyPressCode::NumpadSubtract;
	case VK_ADD: 		return Events::KeyPressCode::NumpadAdd;
	case VK_SEPARATOR: 	return Events::KeyPressCode::NumpadEnter;
	case VK_DECIMAL: 	return Events::KeyPressCode::NumpadDot;

	case VK_F1: return Events::KeyPressCode::F1;
	case VK_F2: return Events::KeyPressCode::F2;
	case VK_F3: return Events::KeyPressCode::F3;
	case VK_F4: return Events::KeyPressCode::F4;
	case VK_F5: return Events::KeyPressCode::F5;
	case VK_F6: return Events::KeyPressCode::F6;
	case VK_F7: return Events::KeyPressCode::F7;
	case VK_F8: return Events::KeyPressCode::F8;
	case VK_F9: return Events::KeyPressCode::F9;
	case VK_F10:return Events::KeyPressCode::F10;
	case VK_F11:return Events::KeyPressCode::F11;
	case VK_F12:return Events::KeyPressCode::F12;
	case VK_F13:return Events::KeyPressCode::F13;
	case VK_F14:return Events::KeyPressCode::F14;
	case VK_F15:return Events::KeyPressCode::F15;
	case VK_F16:return Events::KeyPressCode::F16;
	case VK_F17:return Events::KeyPressCode::F17;
	case VK_F18:return Events::KeyPressCode::F18;
	case VK_F19:return Events::KeyPressCode::F19;
	case VK_F20:return Events::KeyPressCode::F20;
	case VK_F21:return Events::KeyPressCode::F21;
	case VK_F22:return Events::KeyPressCode::F22;
	case VK_F23:return Events::KeyPressCode::F23;
	case VK_F24:return Events::KeyPressCode::F24;

	case 0x30:	return Events::KeyPressCode::Num0;
	case 0x31:	return Events::KeyPressCode::Num1;
	case 0x32:	return Events::KeyPressCode::Num2;
	case 0x33:	return Events::KeyPressCode::Num3;
	case 0x34:	return Events::KeyPressCode::Num4;
	case 0x35:	return Events::KeyPressCode::Num5;
	case 0x36:	return Events::KeyPressCode::Num6;
	case 0x37:	return Events::KeyPressCode::Num7;
	case 0x38:	return Events::KeyPressCode::Num8;
	case 0x39:	return Events::KeyPressCode::Num9;
	case VK_OEM_MINUS:	return Events::KeyPressCode::Dash;
	case VK_OEM_PLUS:	return Events::KeyPressCode::Add;

	case VK_INSERT:		return Events::KeyPressCode::Insert;
	case VK_HOME:		return Events::KeyPressCode::Home;
	case VK_NEXT:	return Events::KeyPressCode::PageUp;
	case VK_PRIOR:	return Events::KeyPressCode::PageDown;
	case VK_END:		return Events::KeyPressCode::End;
	case VK_DELETE:	return Events::KeyPressCode::Delete;
	case VK_PAUSE:		return Events::KeyPressCode::Pause;
	case VK_CAPITAL:	return Events::KeyPressCode::CapsLock;
	case VK_SCROLL:	return Events::KeyPressCode::ScrollLock;

	case VK_OEM_COMMA:		return Events::KeyPressCode::Comma;
	case VK_OEM_PERIOD:		return Events::KeyPressCode::Period;
	case VK_OEM_2:			return Events::KeyPressCode::ForwardSlash;
	case VK_OEM_5:			return Events::KeyPressCode::BackSlash;
	case VK_OEM_1:			return Events::KeyPressCode::Semicolon;
	case VK_OEM_7:			return Events::KeyPressCode::Apostrophe;
	case VK_OEM_4:			return Events::KeyPressCode::LeftBracket;
	case VK_OEM_6:			return Events::KeyPressCode::RightBracket;

	case VK_RETURN:			return Events::KeyPressCode::Enter;
	case VK_BACK:			return Events::KeyPressCode::Backspace;
	case VK_OEM_3:			return Events::KeyPressCode::Tilde;
	case VK_LWIN:			return Events::KeyPressCode::Window;
	case VK_RWIN:			return Events::KeyPressCode::Window;
	}

	return Events::KeyPressCode::None;
}