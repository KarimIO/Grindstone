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

Input::KeyAction translateKey(int key);

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
	Input::Interface* input;
	if (engine_core_) {
		input = engine_core_->getInputManager();
		if (!input) {
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	else {
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	switch (msg) {
	case WM_SIZE:
		input->resizeEvent(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOUSEMOVE:
		input->setMousePosition(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_SETFOCUS:
		input->setFocused(true);
		break;
	case WM_KILLFOCUS:
		input->setFocused(false);
		break;
	case WM_MOUSEHWHEEL:
		if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
			input->setMouseButton(Input::MouseAction::WheelLeft, Input::ButtonStatus::Pressed);
		else
			input->setMouseButton(Input::MouseAction::WheelRight, Input::ButtonStatus::Pressed);
		break;
	case WM_MOUSEWHEEL:
		if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			input->setMouseButton(Input::MouseAction::WheelUp, Input::ButtonStatus::Pressed);
		else
			input->setMouseButton(Input::MouseAction::WheelDown, Input::ButtonStatus::Pressed);
		break;
	case WM_LBUTTONDOWN:
		input->setMouseButton(Input::MouseAction::Left, Input::ButtonStatus::Pressed);
		break;
	case WM_LBUTTONUP:
		input->setMouseButton(Input::MouseAction::Left, Input::ButtonStatus::Released);
		break;
	case WM_MBUTTONDOWN:
		input->setMouseButton(Input::MouseAction::Middle, Input::ButtonStatus::Pressed);
		break;
	case WM_MBUTTONUP:
		input->setMouseButton(Input::MouseAction::Middle, Input::ButtonStatus::Released);
		break;
	case WM_RBUTTONDOWN:
		input->setMouseButton(Input::MouseAction::Right, Input::ButtonStatus::Pressed);
		break;
	case WM_RBUTTONUP:
		input->setMouseButton(Input::MouseAction::Right, Input::ButtonStatus::Released);
		break;
	case WM_XBUTTONDOWN:
		input->setMouseButton(Input::MouseAction::Mouse4, Input::ButtonStatus::Pressed); //MOUSE_MOUSE5 as well
		break;
	case WM_XBUTTONUP:
		input->setMouseButton(Input::MouseAction::Mouse4, Input::ButtonStatus::Released);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		// Repeat Count
		if ((HIWORD(lParam) & KF_REPEAT) == 0) {
			input->setKey(translateKey(int(wParam)), Input::ButtonStatus::Pressed);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		input->setKey(translateKey(int(wParam)), Input::ButtonStatus::Released);
		break;
	case WM_CREATE:
		input->setFocused(true);
		break;
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

bool Win32Window::initialize(CreateInfo& create_info) {
	auto hInstance = GetModuleHandle(NULL);

	should_close_ = false;
	width_ = create_info.width;
	height_ = create_info.height;
	fullscreen_mode_ = create_info.fullscreen;
	engine_core_ = create_info.engine_core;
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

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
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

Input::KeyAction translateKey(int key) {
	//static int keyCodes[] = { KEY_BACKSPACE, KEY_TAB, KEY_NONE, KEY_NONE, KEY_ENTER, KEY_NONE, KEY_NONE, KEY_SHIFT, KEY_CONTROL, KEY_ALT, KEY_PAUSE, KEY_CAPSLOCK, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_ESCAPE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_SPACE, KEY_PG_UP, KEY_PG_DOWN, KEY_END, KEY_HOME, KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN, KEY_SELECT, KEY_PRINT, KEY_EXECUTE, KEY_PRINTSCR, KEY_DELETE, KEY_HELP, KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_NONE, KEY_NONE, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_LWINDOW, KEY_WINDOW, KEY_APPS, KEY_NONE, KEY_NONE, KEY_NUMPAD_0, KEY_NUMPAD_1, KEY_NUMPAD_2, KEY_NUMPAD_3, KEY_NUMPAD_4, KEY_NUMPAD_5, KEY_NUMPAD_6, KEY_NUMPAD_7, KEY_NUMPAD_8, KEY_NUMPAD_9, KEY_NUMPAD_MULTIPLY, KEY_NUMPAD_ADD, KEY_NUMPAD_ENTER, KEY_NUMPAD_SUBTRACT, KEY_NUMPAD_DOT, KEY_NUMPAD_DIVIDE, KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18, KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24, KEY_NONE, KEY_NUMPAD_NUMLOCK, KEY_SCROLL_LOCK, KEY_NONE, KEY_LSHIFT, KEY_LCONTROL, KEY_CONTROL, KEY_LALT, KEY_ALT, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE, KEY_NONE };
	switch (key) {
	case 0x41: return Input::KeyAction::A;
	case 0x42: return Input::KeyAction::B;
	case 0x43: return Input::KeyAction::C;
	case 0x44: return Input::KeyAction::D;
	case 0x45: return Input::KeyAction::E;
	case 0x46: return Input::KeyAction::F;
	case 0x47: return Input::KeyAction::G;
	case 0x48: return Input::KeyAction::H;
	case 0x49: return Input::KeyAction::I;
	case 0x4A: return Input::KeyAction::J;
	case 0x4B: return Input::KeyAction::K;
	case 0x4C: return Input::KeyAction::L;
	case 0x4D: return Input::KeyAction::M;
	case 0x4E: return Input::KeyAction::N;
	case 0x4F: return Input::KeyAction::O;
	case 0x50: return Input::KeyAction::P;
	case 0x51: return Input::KeyAction::Q;
	case 0x52: return Input::KeyAction::R;
	case 0x53: return Input::KeyAction::S;
	case 0x54: return Input::KeyAction::T;
	case 0x55: return Input::KeyAction::U;
	case 0x56: return Input::KeyAction::V;
	case 0x57: return Input::KeyAction::W;
	case 0x58: return Input::KeyAction::X;
	case 0x59: return Input::KeyAction::Y;
	case 0x5A: return Input::KeyAction::Z;

	case VK_ESCAPE: return Input::KeyAction::Escape;
	case VK_TAB: return Input::KeyAction::Tab;
	case VK_SPACE: return Input::KeyAction::Space;
	case VK_LCONTROL: return Input::KeyAction::LeftControl;
	case VK_CONTROL: return Input::KeyAction::Control;
	case VK_LSHIFT: return Input::KeyAction::LeftShift;
	case VK_SHIFT: return Input::KeyAction::Shift;
	case VK_LMENU: return Input::KeyAction::LeftAlt;
	case VK_MENU: return Input::KeyAction::Alt;

	case VK_LEFT:	return Input::KeyAction::ArrowLeft;
	case VK_RIGHT:	return Input::KeyAction::ArrowRight;
	case VK_UP:	return Input::KeyAction::ArrowUp;
	case VK_DOWN:	return Input::KeyAction::ArrowDown;

	case VK_NUMPAD0: return Input::KeyAction::Numpad0;
	case VK_NUMPAD1: return Input::KeyAction::Numpad1;
	case VK_NUMPAD2: return Input::KeyAction::Numpad2;
	case VK_NUMPAD3: return Input::KeyAction::Numpad3;
	case VK_NUMPAD4: return Input::KeyAction::Numpad4;
	case VK_NUMPAD5: return Input::KeyAction::Numpad5;
	case VK_NUMPAD6: return Input::KeyAction::Numpad6;
	case VK_NUMPAD7: return Input::KeyAction::Numpad7;
	case VK_NUMPAD8: return Input::KeyAction::Numpad8;
	case VK_NUMPAD9: return Input::KeyAction::Numpad9;

	case VK_NUMLOCK:	return Input::KeyAction::NumpadLock;
	case VK_DIVIDE: 	return Input::KeyAction::NumpadDivide;
	case VK_MULTIPLY: 	return Input::KeyAction::NumpadMultiply;
	case VK_SUBTRACT: 	return Input::KeyAction::NumpadSubtract;
	case VK_ADD: 		return Input::KeyAction::NumpadAdd;
	case VK_SEPARATOR: 	return Input::KeyAction::NumpadEnter;
	case VK_DECIMAL: 	return Input::KeyAction::NumpadDot;

	case VK_F1: return Input::KeyAction::F1;
	case VK_F2: return Input::KeyAction::F2;
	case VK_F3: return Input::KeyAction::F3;
	case VK_F4: return Input::KeyAction::F4;
	case VK_F5: return Input::KeyAction::F5;
	case VK_F6: return Input::KeyAction::F6;
	case VK_F7: return Input::KeyAction::F7;
	case VK_F8: return Input::KeyAction::F8;
	case VK_F9: return Input::KeyAction::F9;
	case VK_F10:return Input::KeyAction::F10;
	case VK_F11:return Input::KeyAction::F11;
	case VK_F12:return Input::KeyAction::F12;
	case VK_F13:return Input::KeyAction::F13;
	case VK_F14:return Input::KeyAction::F14;
	case VK_F15:return Input::KeyAction::F15;
	case VK_F16:return Input::KeyAction::F16;
	case VK_F17:return Input::KeyAction::F17;
	case VK_F18:return Input::KeyAction::F18;
	case VK_F19:return Input::KeyAction::F19;
	case VK_F20:return Input::KeyAction::F20;
	case VK_F21:return Input::KeyAction::F21;
	case VK_F22:return Input::KeyAction::F22;
	case VK_F23:return Input::KeyAction::F23;
	case VK_F24:return Input::KeyAction::F24;

	case 0x30:	return Input::KeyAction::Num0;
	case 0x31:	return Input::KeyAction::Num1;
	case 0x32:	return Input::KeyAction::Num2;
	case 0x33:	return Input::KeyAction::Num3;
	case 0x34:	return Input::KeyAction::Num4;
	case 0x35:	return Input::KeyAction::Num5;
	case 0x36:	return Input::KeyAction::Num6;
	case 0x37:	return Input::KeyAction::Num7;
	case 0x38:	return Input::KeyAction::Num8;
	case 0x39:	return Input::KeyAction::Num9;
	case VK_OEM_MINUS:	return Input::KeyAction::DASH;
	case VK_OEM_PLUS:	return Input::KeyAction::ADD;

	case VK_INSERT:		return Input::KeyAction::Insert;
	case VK_HOME:		return Input::KeyAction::Home;
	case VK_NEXT:	return Input::KeyAction::PageUp;
	case VK_PRIOR:	return Input::KeyAction::PageDown;
	case VK_END:		return Input::KeyAction::End;
	case VK_DELETE:	return Input::KeyAction::Delete;
	case VK_PAUSE:		return Input::KeyAction::Pause;
	case VK_CAPITAL:	return Input::KeyAction::CapsLock;
	case VK_SCROLL:	return Input::KeyAction::ScrollLock;

	case VK_OEM_COMMA:		return Input::KeyAction::Comma;
	case VK_OEM_PERIOD:		return Input::KeyAction::Period;
	case VK_OEM_2:			return Input::KeyAction::ForwardSlash;
	case VK_OEM_5:			return Input::KeyAction::BackSlash;
	case VK_OEM_1:			return Input::KeyAction::Semicolon;
	case VK_OEM_7:			return Input::KeyAction::Apostrophe;
	case VK_OEM_4:			return Input::KeyAction::LeftBracket;
	case VK_OEM_6:			return Input::KeyAction::RightBracket;

	case VK_RETURN:			return Input::KeyAction::Enter;
	case VK_BACK:			return Input::KeyAction::Backspace;
	case VK_OEM_3:			return Input::KeyAction::Tilde;
	case VK_LWIN:			return Input::KeyAction::Window;
	case VK_RWIN:			return Input::KeyAction::Window;
	}

	return Input::KeyAction::None;
}