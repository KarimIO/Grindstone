#include <iostream>
#include <assert.h>
#include <vector>
#include <tchar.h>
#include <Windowsx.h>
#include <ShlObj_core.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <GL/wglext.h>

#include <EngineCore/Logger.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <Common/Input/InputInterface.hpp>

#include "GlfwWindow.hpp"

using namespace Grindstone;
using namespace Grindstone::Memory;

static Events::KeyPressCode TranslateKeyboardButton(int key);
static Events::MouseButtonCode TranslateMouseButton(int action);

static Input::Interface* GetInputFromGrindstoneWindow(GlfwWindow* window) {
	if (window && window->engineCore) {
		return window->engineCore->GetInputManager();
	}

	return nullptr;
}

static Input::Interface* GetInput(GLFWwindow* window) {
	GlfwWindow* win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
	if (win) {
		return GetInputFromGrindstoneWindow(win);
	}

	return nullptr;
}

static Grindstone::EngineCore* g_engineCore = nullptr;
static void OnErrorCallback(int error, const char* description) {
	GPRINT_ERROR(LogSource::GraphicsAPI, description);
}

static void OnKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (Input::Interface* input = GetInput(window)) {
		input->SetKeyPressed(TranslateKeyboardButton(key), action != GLFW_RELEASE);
	}
}

static void OnCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	if (Input::Interface* input = GetInput(window)) {
		input->OnMouseMoved(static_cast<int>(xpos), static_cast<int>(ypos));
	}
}

static void OnMouseBtnCallback(GLFWwindow* window, int button, int action, int mods) {
	if (Input::Interface* input = GetInput(window)) {
		input->SetMouseButton(TranslateMouseButton(button), action == GLFW_PRESS);
	}
}

static void OnMouseScrollCallback(GLFWwindow* window, double offsetX, double offsetY) {
	if (Input::Interface* input = GetInput(window)) {
		input->MouseScroll(static_cast<float>(offsetX), static_cast<float>(offsetY));
	}
}

static void OnWindowPosCallback(GLFWwindow* window, int xpos, int ypos) {
}

static void OnWindowSizeCallback(GLFWwindow* window, int width, int height) {
	GlfwWindow* win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));

	auto wgb = win->GetWindowGraphicsBinding();
	if (wgb && win->IsSwapchainControlledByEngine()) {
		wgb->Resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	}
}

static void OnWindowFocusCallback(GLFWwindow* window, int focused) {
	if (Input::Interface* input = GetInput(window)) {
		input->SetIsFocused(focused == GLFW_TRUE);
	}
}

static void OnWindowMinimizeCallback(GLFWwindow* window, int iconified) {
}

static void OnWindowTryQuit(GLFWwindow* window) {
	GlfwWindow* win = static_cast<GlfwWindow*>(glfwGetWindowUserPointer(window));
	if (Input::Interface* input = GetInputFromGrindstoneWindow(win)) {
		input->TryQuit(win);
	}
}

bool GlfwWindow::CopyStringToClipboard(const std::string& stringToCopy) {
	glfwSetClipboardString(windowHandle, stringToCopy.c_str());

	return true;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	if (uMsg == BFFM_INITIALIZED) {
		std::string tmp = (const char*)lpData;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

std::filesystem::path GlfwWindow::BrowseFolder(std::filesystem::path& defaultPath) {
	std::filesystem::path outPath;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileOpenDialog* pFileOpen = nullptr;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr)) {
			DWORD dwOptions;
			if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
				pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
			}

			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr)) {
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr)) {
						std::wstring tempWStr(pszFilePath);
						outPath = std::filesystem::path(tempWStr);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}

	return outPath;
}

std::filesystem::path GlfwWindow::OpenFileDialogue(const char* filter) {
	OPENFILENAME ofn{};
	char fileName[MAX_PATH] = "";

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window(windowHandle);
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = "";

	if (GetOpenFileName(&ofn)) {
		return fileName;
	}

	return "";
}

std::filesystem::path GlfwWindow::SaveFileDialogue(const char* filter) {
	OPENFILENAME ofn;
	char fileName[MAX_PATH] = "";
	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window(windowHandle);
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = "";

	if (GetSaveFileName(&ofn)) {
		return fileName;
	}

	return "";
}

void GlfwWindow::ExplorePath(const char* path) {
	std::string commandArg = std::string("/select, ") + path;
	ShellExecute(0, NULL, "explorer.exe", commandArg.c_str(), NULL, SW_SHOWNORMAL);
}

void GlfwWindow::OpenFileUsingDefaultProgram(const char* path) {
	ShellExecute(0, 0, path, 0, 0, SW_SHOW);
}


GlfwWindow::~GlfwWindow() {
	AllocatorCore::Free(windowsGraphicsBinding);
}

bool GlfwWindow::Initialize(CreateInfo& createInfo) {
	g_engineCore = createInfo.engineCore;
	glfwSetErrorCallback(&OnErrorCallback);

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	bool isVulkan = true;
	bool isOpenGl = false;
	engineCore = createInfo.engineCore;
	isSwapchainControlledByEngine = createInfo.isSwapchainControlledByEngine;

	if (isVulkan) {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	windowHandle = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, NULL, NULL);

	glfwSetWindowUserPointer(windowHandle, this);
	glfwSetKeyCallback(windowHandle, &OnKeyCallback);
	glfwSetCursorPosCallback(windowHandle, &OnCursorPosCallback);
	glfwSetMouseButtonCallback(windowHandle, &OnMouseBtnCallback);
	glfwSetScrollCallback(windowHandle, &OnMouseScrollCallback);
	glfwSetWindowPosCallback(windowHandle, &OnWindowPosCallback);
	glfwSetFramebufferSizeCallback(windowHandle, &OnWindowSizeCallback);
	glfwSetWindowFocusCallback(windowHandle, &OnWindowFocusCallback);
	glfwSetWindowIconifyCallback(windowHandle, &OnWindowMinimizeCallback);
	glfwSetWindowCloseCallback(windowHandle, &OnWindowTryQuit);

	if (isOpenGl) {
		glfwMakeContextCurrent(windowHandle);
		glfwSwapInterval(1);
	}

	return true;
}

void GlfwWindow::Show() {
	glfwShowWindow(windowHandle);
}

void GlfwWindow::Hide() {
	glfwHideWindow(windowHandle);
}

bool GlfwWindow::ShouldClose() {
	return glfwWindowShouldClose(windowHandle);
}

void GlfwWindow::HandleEvents() {
	glfwPollEvents();
}

void GlfwWindow::SetFullscreen(FullscreenMode mode) {
	// TODO: Handle this

}

void GlfwWindow::GetWindowRect(unsigned int &left, unsigned int &top, unsigned int &right, unsigned int &bottom) const {
	int l, t, r, b;
	glfwGetWindowFrameSize(windowHandle, &l, &t, &r, &b);

	left = static_cast<unsigned int>(l);
	right = static_cast<unsigned int>(r);
	top = static_cast<unsigned int>(t);
	bottom = static_cast<unsigned int>(b);
}

void GlfwWindow::GetWindowSize(unsigned int &width, unsigned int &height) const {
	int w, h;
	glfwGetWindowSize(windowHandle, &w, &h);
	width = w;
	height = h;
}

void GlfwWindow::SetWindowSize(unsigned int width, unsigned int height) {
	glfwSetWindowSize(windowHandle, width, height);
}

void GlfwWindow::SetMousePos(unsigned int x, unsigned int y) {
	glfwSetCursorPos(windowHandle, static_cast<double>(x), static_cast<double>(y));
}

void GlfwWindow::GetMousePos(unsigned int& x, unsigned int& y) const {
	double xpos, ypos;
	glfwGetCursorPos(windowHandle, &xpos, &ypos);

	x = static_cast<unsigned int>(xpos);
	y = static_cast<unsigned int>(ypos);
}

void GlfwWindow::SetCursorMode(Grindstone::Input::CursorMode cursorMode) {
	const int cursorModes[] = { GLFW_CURSOR_NORMAL, GLFW_CURSOR_HIDDEN, GLFW_CURSOR_DISABLED };
	return glfwSetInputMode(windowHandle, GLFW_CURSOR, cursorModes[(int)cursorMode]);
}

Grindstone::Input::CursorMode GlfwWindow::GetCursorMode() const {
	int mode = glfwGetInputMode(windowHandle, GLFW_CURSOR);

	switch (mode) {
	case GLFW_CURSOR_NORMAL:
		return Grindstone::Input::CursorMode::Normal;
	case GLFW_CURSOR_HIDDEN:
		return Grindstone::Input::CursorMode::Hidden;
	case GLFW_CURSOR_DISABLED:
		return Grindstone::Input::CursorMode::Disabled;
	default:
		GPRINT_FATAL(LogSource::GraphicsAPI, "Invalid CursorMode!");
		return Grindstone::Input::CursorMode::Normal;
	}
}

void GlfwWindow::SetMouseIsRawMotion(bool isRawMotion) {
	if (glfwRawMouseMotionSupported()) {
		glfwSetInputMode(windowHandle, GLFW_RAW_MOUSE_MOTION, isRawMotion ? GLFW_TRUE : GLFW_FALSE);
	}
}

bool GlfwWindow::GetMouseIsRawMotion() const {
	return glfwGetInputMode(windowHandle, GLFW_RAW_MOUSE_MOTION) == GLFW_TRUE;
}

void GlfwWindow::SetWindowPos(unsigned int x, unsigned int y) {
	glfwSetWindowPos(windowHandle, static_cast<int>(x), static_cast<int>(y));
}

void GlfwWindow::GetWindowPos(unsigned int& x, unsigned int& y) const {
	int xpos, ypos;
	glfwGetWindowPos(windowHandle, &xpos, &ypos);
	x = static_cast<unsigned int>(xpos);
	y = static_cast<unsigned int>(ypos);
}

void GlfwWindow::Close() {
	glfwSetWindowShouldClose(windowHandle, GLFW_TRUE);
}

GLFWwindow* GlfwWindow::GetHandle() const {
	return windowHandle;
}

void GlfwWindow::SetWindowFocus(bool isFocused) {
	if (windowHandle) {
		glfwFocusWindow(windowHandle);
	}
}

bool GlfwWindow::GetWindowFocus() const {
	return glfwGetWindowAttrib(windowHandle, GLFW_FOCUSED) == GLFW_TRUE;
}

bool GlfwWindow::GetWindowMinimized() const {
	return glfwGetWindowAttrib(windowHandle, GLFW_ICONIFIED) == GLFW_TRUE;
}

void GlfwWindow::GetTitle(char* allocatedBuffer) const {
	// TODO: Implement this
}

void GlfwWindow::SetTitle(const char* title) {
	glfwSetWindowTitle(windowHandle, title);
}

void GlfwWindow::SetWindowAlpha(float alpha) {
	glfwSetWindowOpacity(windowHandle, alpha);
}

float GlfwWindow::GetWindowDpiScale() const {
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

void GlfwWindow::OnSwapchainResized(int width, int height) {
	if (Input::Interface* input = GetInputFromGrindstoneWindow(this)) {
		input->ResizeEvent(width, height);
	}
}

static Events::KeyPressCode TranslateKeyboardButton(int key) {
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

	case GLFW_KEY_ESCAPE: return Events::KeyPressCode::Escape;
	case GLFW_KEY_TAB: return Events::KeyPressCode::Tab;
	case GLFW_KEY_SPACE: return Events::KeyPressCode::Space;
	case GLFW_KEY_LEFT_CONTROL: return Events::KeyPressCode::LeftControl;
	case GLFW_KEY_RIGHT_CONTROL: return Events::KeyPressCode::Control;
	case GLFW_KEY_LEFT_SHIFT: return Events::KeyPressCode::LeftShift;
	case GLFW_KEY_RIGHT_SHIFT: return Events::KeyPressCode::Shift;
	case GLFW_KEY_LEFT_ALT: return Events::KeyPressCode::LeftAlt;
	case GLFW_KEY_RIGHT_ALT: return Events::KeyPressCode::Alt;

	case GLFW_KEY_LEFT:	return Events::KeyPressCode::ArrowLeft;
	case GLFW_KEY_RIGHT:	return Events::KeyPressCode::ArrowRight;
	case GLFW_KEY_UP:	return Events::KeyPressCode::ArrowUp;
	case GLFW_KEY_DOWN:	return Events::KeyPressCode::ArrowDown;

	case GLFW_KEY_KP_0: return Events::KeyPressCode::Numpad0;
	case GLFW_KEY_KP_1: return Events::KeyPressCode::Numpad1;
	case GLFW_KEY_KP_2: return Events::KeyPressCode::Numpad2;
	case GLFW_KEY_KP_3: return Events::KeyPressCode::Numpad3;
	case GLFW_KEY_KP_4: return Events::KeyPressCode::Numpad4;
	case GLFW_KEY_KP_5: return Events::KeyPressCode::Numpad5;
	case GLFW_KEY_KP_6: return Events::KeyPressCode::Numpad6;
	case GLFW_KEY_KP_7: return Events::KeyPressCode::Numpad7;
	case GLFW_KEY_KP_8: return Events::KeyPressCode::Numpad8;
	case GLFW_KEY_KP_9: return Events::KeyPressCode::Numpad9;

	case GLFW_KEY_NUM_LOCK:		return Events::KeyPressCode::NumpadLock;
	case GLFW_KEY_KP_DIVIDE: 	return Events::KeyPressCode::NumpadDivide;
	case GLFW_KEY_KP_MULTIPLY: 	return Events::KeyPressCode::NumpadMultiply;
	case GLFW_KEY_KP_SUBTRACT: 	return Events::KeyPressCode::NumpadSubtract;
	case GLFW_KEY_KP_ADD: 		return Events::KeyPressCode::NumpadAdd;
	case GLFW_KEY_KP_ENTER:		return Events::KeyPressCode::NumpadEnter;
	case GLFW_KEY_KP_DECIMAL: 	return Events::KeyPressCode::NumpadDot;

	case GLFW_KEY_F1:	return Events::KeyPressCode::F1;
	case GLFW_KEY_F2:	return Events::KeyPressCode::F2;
	case GLFW_KEY_F3:	return Events::KeyPressCode::F3;
	case GLFW_KEY_F4:	return Events::KeyPressCode::F4;
	case GLFW_KEY_F5:	return Events::KeyPressCode::F5;
	case GLFW_KEY_F6:	return Events::KeyPressCode::F6;
	case GLFW_KEY_F7:	return Events::KeyPressCode::F7;
	case GLFW_KEY_F8:	return Events::KeyPressCode::F8;
	case GLFW_KEY_F9:	return Events::KeyPressCode::F9;
	case GLFW_KEY_F10:	return Events::KeyPressCode::F10;
	case GLFW_KEY_F11:	return Events::KeyPressCode::F11;
	case GLFW_KEY_F12:	return Events::KeyPressCode::F12;
	case GLFW_KEY_F13:	return Events::KeyPressCode::F13;
	case GLFW_KEY_F14:	return Events::KeyPressCode::F14;
	case GLFW_KEY_F15:	return Events::KeyPressCode::F15;
	case GLFW_KEY_F16:	return Events::KeyPressCode::F16;
	case GLFW_KEY_F17:	return Events::KeyPressCode::F17;
	case GLFW_KEY_F18:	return Events::KeyPressCode::F18;
	case GLFW_KEY_F19:	return Events::KeyPressCode::F19;
	case GLFW_KEY_F20:	return Events::KeyPressCode::F20;
	case GLFW_KEY_F21:	return Events::KeyPressCode::F21;
	case GLFW_KEY_F22:	return Events::KeyPressCode::F22;
	case GLFW_KEY_F23:	return Events::KeyPressCode::F23;
	case GLFW_KEY_F24:	return Events::KeyPressCode::F24;

	case GLFW_KEY_0:		return Events::KeyPressCode::Num0;
	case GLFW_KEY_1:		return Events::KeyPressCode::Num1;
	case GLFW_KEY_2:		return Events::KeyPressCode::Num2;
	case GLFW_KEY_3:		return Events::KeyPressCode::Num3;
	case GLFW_KEY_4:		return Events::KeyPressCode::Num4;
	case GLFW_KEY_5:		return Events::KeyPressCode::Num5;
	case GLFW_KEY_6:		return Events::KeyPressCode::Num6;
	case GLFW_KEY_7:		return Events::KeyPressCode::Num7;
	case GLFW_KEY_8:		return Events::KeyPressCode::Num8;
	case GLFW_KEY_9:		return Events::KeyPressCode::Num9;
	case GLFW_KEY_MINUS:	return Events::KeyPressCode::Dash;
	case GLFW_KEY_EQUAL:	return Events::KeyPressCode::Add;

	case GLFW_KEY_INSERT:		return Events::KeyPressCode::Insert;
	case GLFW_KEY_HOME:			return Events::KeyPressCode::Home;
	case GLFW_KEY_PAGE_UP:		return Events::KeyPressCode::PageUp;
	case GLFW_KEY_PAGE_DOWN:	return Events::KeyPressCode::PageDown;
	case GLFW_KEY_END:			return Events::KeyPressCode::End;
	case GLFW_KEY_DELETE:		return Events::KeyPressCode::Delete;
	case GLFW_KEY_PAUSE:		return Events::KeyPressCode::Pause;
	case GLFW_KEY_CAPS_LOCK:	return Events::KeyPressCode::CapsLock;
	case GLFW_KEY_SCROLL_LOCK:	return Events::KeyPressCode::ScrollLock;

	case GLFW_KEY_COMMA:		return Events::KeyPressCode::Comma;
	case GLFW_KEY_PERIOD:		return Events::KeyPressCode::Period;
	case GLFW_KEY_SLASH:		return Events::KeyPressCode::ForwardSlash;
	case GLFW_KEY_BACKSLASH:	return Events::KeyPressCode::BackSlash;
	case GLFW_KEY_SEMICOLON:	return Events::KeyPressCode::Semicolon;
	case GLFW_KEY_APOSTROPHE:	return Events::KeyPressCode::Apostrophe;
	case GLFW_KEY_LEFT_BRACKET:	return Events::KeyPressCode::LeftBracket;
	case GLFW_KEY_RIGHT_BRACKET:return Events::KeyPressCode::RightBracket;

	case GLFW_KEY_ENTER:		return Events::KeyPressCode::Enter;
	case GLFW_KEY_BACKSPACE:	return Events::KeyPressCode::Backspace;
	case GLFW_KEY_GRAVE_ACCENT:	return Events::KeyPressCode::Tilde;
	case GLFW_KEY_LEFT_SUPER:	return Events::KeyPressCode::Window;
	case GLFW_KEY_RIGHT_SUPER:	return Events::KeyPressCode::Window;
	}

	return Events::KeyPressCode::Invalid;
}

static Events::MouseButtonCode TranslateMouseButton(int action) {
	switch (action) {
	case GLFW_MOUSE_BUTTON_LEFT:		return Events::MouseButtonCode::Left;
	case GLFW_MOUSE_BUTTON_RIGHT:		return Events::MouseButtonCode::Right;
	case GLFW_MOUSE_BUTTON_MIDDLE:		return Events::MouseButtonCode::Middle;
	case GLFW_MOUSE_BUTTON_4:			return Events::MouseButtonCode::Mouse4;
	case GLFW_MOUSE_BUTTON_5:			return Events::MouseButtonCode::Mouse5;

	// TODO: Support 6 7 8?
	default:
	case GLFW_MOUSE_BUTTON_6:
	case GLFW_MOUSE_BUTTON_7:
	case GLFW_MOUSE_BUTTON_8:
		return Events::MouseButtonCode::Invalid;
	}
}
