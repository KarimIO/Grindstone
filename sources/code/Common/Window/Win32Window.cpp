//#include "../Engine/Core/InputInterface.hpp"
#include "Win32Window.hpp"
#include <GL/gl3w.h>
#include <GL/wglext.h>
#include <Windowsx.h>

#include <iostream>
#include <assert.h>
#include <vector>

#include <tchar.h>

using namespace Grindstone;

int TranslateKey(int key);

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
	//if (!input_)
	//	return DefWindowProc(hwnd, msg, wParam, lParam);

	switch (msg) {
	/*case WM_SIZE:
		input_->ResizeEvent(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		input_->SetMousePosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_SETFOCUS:
		input_->SetFocused(true);
		break;
	case WM_KILLFOCUS:
		input_->SetFocused(false);
		break;
	case WM_MOUSEHWHEEL:
		if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
			input_->SetMouseButton(MOUSE_WHEEL_LEFT, true);
		else
			input_->SetMouseButton(MOUSE_WHEEL_RIGHT, true);
		break;
	case WM_MOUSEWHEEL:
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			input_->SetMouseButton(MOUSE_WHEEL_UP, true);
		else
			input_->SetMouseButton(MOUSE_WHEEL_DOWN, true);
		break;
	case WM_LBUTTONDOWN:
		input_->SetMouseButton(MOUSE_LEFT, true);
		break;
	case WM_LBUTTONUP:
		input_->SetMouseButton(MOUSE_LEFT, false);
		break;
	case WM_MBUTTONDOWN:
		input_->SetMouseButton(MOUSE_MIDDLE, true);
		break;
	case WM_MBUTTONUP:
		input_->SetMouseButton(MOUSE_MIDDLE, false);
		break;
	case WM_RBUTTONDOWN:
		input_->SetMouseButton(MOUSE_RIGHT, true);
		break;
	case WM_RBUTTONUP:
		input_->SetMouseButton(MOUSE_RIGHT, false);
		break;
	case WM_XBUTTONDOWN:
		input_->SetMouseButton(MOUSE_MOUSE4, true); //MOUSE_MOUSE5 as well
		break;
	case WM_XBUTTONUP:
		input_->SetMouseButton(MOUSE_MOUSE4, false);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		// Repeat Count
		if ((HIWORD(lParam) & KF_REPEAT) == 0) {
			input_->SetKey(TranslateKey(int(wParam)), true);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		input_->SetKey(TranslateKey(int(wParam)), false);
		break;
	case WM_CREATE:
		input_->SetFocused(true);
		break;*/
	case WM_CLOSE:
		if (MessageBox(NULL, _T("Are you sure you want to cancel?"), _T("Error!"), MB_ICONEXCLAMATION | MB_YESNO)) {
			should_close_ = true;
			DestroyWindow(hwnd);
		}
		break;
	case WM_DESTROY:
		should_close_ = true;
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool Win32Window::initialize(CreateInfo create_info) {
	auto hInstance = GetModuleHandle(NULL);

	should_close_ = false;
	width_ = create_info.width;
	height_ = create_info.height;
	fullscreen_mode_ = create_info.fullscreen;
	input_ = create_info.input_interface;
	auto d = create_info.display;

	ex_style_ = 0;

	switch (fullscreen_mode_) {
	case FullscreenMode::Borderless: {
		window_size_ = RECT{ (LONG)d.x_, (LONG)d.y_, (LONG)(d.x_ + d.width_), (LONG)(d.y_ + d.height_) };
		style_ = WS_POPUP;
		break;
	}
	case FullscreenMode::Fullscreen: {
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsHeight = (unsigned long)height_;
		dmScreenSettings.dmPelsWidth = (unsigned long)width_;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		dmScreenSettings.dmPosition.x = d.x_;
		dmScreenSettings.dmPosition.y = d.y_;

		window_size_.left = 0;
		window_size_.top = 0;
		window_size_.right = (unsigned long)width_;
		window_size_.bottom = (unsigned long)height_;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
		style_ = WS_POPUP;
		break;
	}
	case FullscreenMode::Windowed:
		window_size_.left = (GetSystemMetrics(SM_CXSCREEN) - width_) / 2;
		window_size_.top = (GetSystemMetrics(SM_CYSCREEN) - height_) / 2;
		window_size_.right = window_size_.left + width_;
		window_size_.bottom = window_size_.top + height_;
		style_ = WS_OVERLAPPEDWINDOW;
		break;
	}

	AdjustWindowRect(&window_size_, style_, FALSE);

	/*int n = ::MultiByteToWideChar(CP_UTF8, 0, create_info.title, -1, NULL, 0);
	std::vector<wchar_t> title_w;
	title_w.resize(n);
	::MultiByteToWideChar(CP_UTF8, 0, create_info.title, -1, title_w.data(), n);*/

	WNDCLASSEX wc;
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
	wc.lpszClassName = create_info.title;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, _T("Error registering class"),
			_T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	window_handle_ = CreateWindowEx(ex_style_,
		create_info.title,
		create_info.title,
		style_,
		window_size_.left,
		window_size_.top,
		window_size_.right - window_size_.left,
		window_size_.bottom - window_size_.top,
		NULL,
		NULL,
		hInstance,
		this);

	if (!window_handle_) {
		MessageBox(NULL, _T("Error creating window"),
			_T("Error"), MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

void Win32Window::show() {
	ShowWindow(window_handle_, SW_SHOW);
	UpdateWindow(window_handle_);
}

bool Win32Window::shouldClose() {
	return should_close_;
}

bool Win32Window::handleEvents() {
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		// Translate the message and dispatch it to WindowProc()
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			return false;
	}

	return true;
}

void Win32Window::setFullscreen(FullscreenMode mode) {
	fullscreen_mode_ = mode;

	DWORD style;

	switch (fullscreen_mode_) {
	case FullscreenMode::Windowed: {
		window_size_.left = (GetSystemMetrics(SM_CXSCREEN) - width_) / 2;
		window_size_.top = (GetSystemMetrics(SM_CYSCREEN) - height_) / 2;
		window_size_.right = window_size_.left + width_;
		window_size_.bottom = window_size_.top + height_;
		style = WS_VISIBLE | WS_OVERLAPPEDWINDOW;

		SetWindowLongPtr(window_handle_, GWL_STYLE, style);
		SetWindowPos(window_handle_, NULL, 0, 0, width_, height_, SWP_FRAMECHANGED);
		break;
	}
	case FullscreenMode::Fullscreen:
	{
		// TODO: Add devmode stuff
		window_size_.left = 0;
		window_size_.top = 0;
		window_size_.right = width_;
		window_size_.bottom = height_;
		style = WS_VISIBLE | WS_POPUP;

		SetWindowLongPtr(window_handle_, GWL_STYLE, WS_VISIBLE | WS_POPUP);
		SetWindowPos(window_handle_, HWND_TOP, 0, 0, window_size_.right, window_size_.bottom, SWP_FRAMECHANGED);
		break;
	}
	default:
	case FullscreenMode::Borderless:
	{
		window_size_.left = 0;
		window_size_.top = 0;
		window_size_.right = GetSystemMetrics(SM_CXSCREEN);
		window_size_.bottom = GetSystemMetrics(SM_CYSCREEN);
		style = WS_VISIBLE | WS_POPUP;

		SetWindowLongPtr(window_handle_, GWL_STYLE, WS_VISIBLE | WS_POPUP);
		SetWindowPos(window_handle_, HWND_TOP, 0, 0, window_size_.right, window_size_.bottom, SWP_FRAMECHANGED);
		break;
	}
	}

	AdjustWindowRect(&window_size_, style, FALSE);
}


void Win32Window::getWindowRect(unsigned int &left, unsigned int &top, unsigned int &right, unsigned int &bottom) {
	RECT rect;
	GetWindowRect(window_handle_, &rect);
	left = rect.left;
	top = rect.top;
	right = rect.right;
	bottom = rect.bottom;
}

void Win32Window::getWindowSize(unsigned int &width, unsigned int &height) {
	RECT rect;
	::GetClientRect(window_handle_, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;
}

void Win32Window::setWindowSize(unsigned int width, unsigned int height) {
	RECT rect = { 0, 0, (LONG)width, (LONG)height };
	::AdjustWindowRectEx(&rect, style_, FALSE, ex_style_); // Client to Screen
	::SetWindowPos(window_handle_, NULL, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

}

void Win32Window::setMousePos(unsigned int x, unsigned int y) {
	POINT pt;
	pt.x = (LONG)x;
	pt.y = (LONG)y;
	ClientToScreen(window_handle_, &pt);
	SetCursorPos(pt.x, pt.y);
}

void Win32Window::getMousePos(unsigned int& x, unsigned int& y) {
	POINT point;
	GetCursorPos(&point);

	x = point.x;
	y = point.y;
}

void Win32Window::setWindowPos(unsigned int x, unsigned int y) {
	RECT rect = { (LONG)x, (LONG)y, (LONG)x, (LONG)y };
	::AdjustWindowRectEx(&rect, style_, FALSE, ex_style_);
	::SetWindowPos(window_handle_, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void Win32Window::getWindowPos(unsigned int& width, unsigned int& height) {
	POINT pos = { 0, 0 };
	::ClientToScreen(window_handle_, &pos);
	width = pos.x;
	height = pos.y;
}

void Win32Window::close() {
	DestroyWindow(window_handle_);
	// UnregisterClassA("GameWindow", GetModuleHandle(NULL));
}

HWND Win32Window::getHandle() {
	return window_handle_;
}

void Win32Window::setWindowFocus() {
	::BringWindowToTop(window_handle_);
	::SetForegroundWindow(window_handle_);
	::SetFocus(window_handle_);
}

bool Win32Window::getWindowFocus() {
	return ::GetForegroundWindow() == window_handle_;
}

bool Win32Window::getWindowMinimized() {
	return ::IsIconic(window_handle_) != 0;
}

void Win32Window::setWindowTitle(const char* title) {
	// ::SetWindowTextA() doesn't properly handle UTF-8 so we explicitely convert our string.
	int n = ::MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
	std::vector<wchar_t> title_w;
	title_w.resize(n);
	::MultiByteToWideChar(CP_UTF8, 0, title, -1, title_w.data(), n);
	::SetWindowTextW(window_handle_, title_w.data());
}

void Win32Window::setWindowAlpha(float alpha) {
	//IM_ASSERT(alpha >= 0.0f && alpha <= 1.0f);
	if (alpha < 1.0f) {
		DWORD style = ::GetWindowLongW(window_handle_, GWL_EXSTYLE) | WS_EX_LAYERED;
		::SetWindowLongW(window_handle_, GWL_EXSTYLE, style);
		::SetLayeredWindowAttributes(window_handle_, 0, (BYTE)(255 * alpha), LWA_ALPHA);
	}
	else {
		DWORD style = ::GetWindowLongW(window_handle_, GWL_EXSTYLE) & ~WS_EX_LAYERED;
		::SetWindowLongW(window_handle_, GWL_EXSTYLE, style);
	}
}

float Win32Window::getWindowDpiScale() {
	/*HMONITOR monitor = ::MonitorFromWindow(window_handle_, MONITOR_DEFAULTTONEAREST);
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

int TranslateKey(int key) {
	//static int keyCodes[] = { KEY_BACKSPACE, KEY_TAB, KEY_NONE, KEY_NONE, KEY_ENTER, KEY_NONE, KEY_NONE, KEY_SHIFT, KEY_CONTROL, KEY_ALT, KEY_PAUSE, KEY_CAPSLOCK, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_ESCAPE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_SPACE, KEY_PG_UP, KEY_PG_DOWN, KEY_END, KEY_HOME, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_SELECT, KEY_PRINT, KEY_EXECUTE, KEY_PRINTSCR, KEY_DELETE, KEY_HELP, KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_NONE, KEY_NONE, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_LWINDOW, KEY_WINDOW, KEY_APPS, KEY_NONE, KEY_NONE, KEY_NUMPAD_0, KEY_NUMPAD_1, KEY_NUMPAD_2, KEY_NUMPAD_3, KEY_NUMPAD_4, KEY_NUMPAD_5, KEY_NUMPAD_6, KEY_NUMPAD_7, KEY_NUMPAD_8, KEY_NUMPAD_9, KEY_NUMPAD_MULTIPLY, KEY_NUMPAD_ADD, KEY_NUMPAD_ENTER, KEY_NUMPAD_SUBTRACT, KEY_NUMPAD_DOT, KEY_NUMPAD_DIVIDE, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_NONE, KEY_NUMPAD_NUMLOCK, KEY_SCROLL_LOCK, KEY_NONE, KEY_LSHIFT, KEY_LCONTROL, KEY_CONTROL, KEY_LALT, KEY_ALT, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE };
	/*switch (key) {
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
	}*/

	return -1;
}