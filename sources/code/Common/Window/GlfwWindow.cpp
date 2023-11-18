//#include "../Engine/Core/InputInterface.hpp"
#include "GlfwWindow.hpp"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <EngineCore/EngineCore.hpp>
#include <Common/Input.hpp>
#include <GL/wglext.h>
#include <Windowsx.h>
#include <ShlObj_core.h>

#include <iostream>
#include <assert.h>
#include <vector>

#include <tchar.h>

using namespace Grindstone;

Events::KeyPressCode TranslateKey(int key);

/*
LRESULT CALLBACK GlfwWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	Input::Interface* input = nullptr;
	if (engineCore) {
		input = engineCore->GetInputManager();
		if (input == nullptr) {
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}
	}
	else {
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	switch (msg) {
	case WM_SIZE: {
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);
		input->ResizeEvent(width, height);
		windowsGraphicsBinding->Resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		break;
	}
	case WM_MOUSEMOVE:
		input->OnMouseMoved(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_SETFOCUS:
		input->SetIsFocused(true);
		break;
	case WM_KILLFOCUS:
		input->SetIsFocused(false);
		break;
	case WM_MOUSEHWHEEL: {
		float delta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
		input->MouseScroll(delta, 0);
		break;
	}
	case WM_MOUSEWHEEL: {
		float delta = (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
		input->MouseScroll(0, delta);
		break;
	}
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
		break;
	case WM_DESTROY:
		input->ForceQuit(this);
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
*/

void OnErrorCallback(int error, const char* description){
	fprintf(stderr, "Error: %s\n", description);
}

void OnKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// input->SetKeyPressed(TranslateKey(int(wParam)), false);
}

void OnCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {

}

void OnMouseBtnCallback(GLFWwindow* window, int button, int action, int mods) {

}

void OnWindowPosCallback(GLFWwindow* window, int xpos, int ypos) {

}

void OnWindowSizeCallback(GLFWwindow* window, int width, int height) {

}

void OnWindowFocusCallback(GLFWwindow* window, int focused) {
	// input->SetIsFocused(focused);
}

void OnWindowMinimizeCallback(GLFWwindow* window, int iconified) {

}

void OnWindowTryQuit(GLFWwindow* window) {
	/*input->TryQuit(this);
	DestroyWindow(hwnd);*/
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

bool GlfwWindow::Initialize(CreateInfo& createInfo) {
	auto hInstance = GetModuleHandle(NULL);

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwSetErrorCallback(&OnErrorCallback);

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	bool isVulkan = true;
	bool isOpenGl = false;

	if (isVulkan) {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	windowHandle = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, NULL, NULL);

	glfwSetKeyCallback(windowHandle, &OnKeyCallback);
	glfwSetCursorPosCallback(windowHandle, &OnCursorPosCallback);
	glfwSetMouseButtonCallback(windowHandle, &OnMouseBtnCallback);
	glfwSetWindowPosCallback(windowHandle, &OnWindowPosCallback);
	glfwSetFramebufferSizeCallback(windowHandle, &OnWindowSizeCallback);
	glfwSetWindowFocusCallback(windowHandle, &OnWindowFocusCallback);
	glfwSetWindowIconifyCallback(windowHandle, &OnWindowMinimizeCallback);

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
	MSG eventMessage;

	while (PeekMessage(&eventMessage, NULL, 0, 0, PM_REMOVE)) {
		if (eventMessage.message == WM_QUIT) {
			return;
		}

		TranslateMessage(&eventMessage);
		DispatchMessage(&eventMessage);
	}
}

void GlfwWindow::SetFullscreen(FullscreenMode mode) {
	// TODO: Handle this
	
}


void GlfwWindow::GetWindowRect(unsigned int &left, unsigned int &top, unsigned int &right, unsigned int &bottom) {
	int l, t, r, b;
	glfwGetWindowFrameSize(windowHandle, &l, &t, &r, &b);

	left = static_cast<unsigned int>(l);
	right = static_cast<unsigned int>(r);
	top = static_cast<unsigned int>(t);
	bottom = static_cast<unsigned int>(b);
}

void GlfwWindow::GetWindowSize(unsigned int &width, unsigned int &height) {
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

void GlfwWindow::GetMousePos(unsigned int& x, unsigned int& y) {
	double xpos, ypos;
	glfwGetCursorPos(windowHandle, &xpos, &ypos);

	x = static_cast<unsigned int>(xpos);
	y = static_cast<unsigned int>(ypos);
}

void GlfwWindow::SetCursorIsVisible(bool isVisible) {
	glfwSetInputMode(windowHandle, GLFW_CURSOR, isVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

bool GlfwWindow::GetCursorIsVisible() {
	return glfwGetInputMode(windowHandle, GLFW_CURSOR) == GLFW_CURSOR_NORMAL;
}

void GlfwWindow::SetWindowPos(unsigned int x, unsigned int y) {
	glfwSetWindowPos(windowHandle, static_cast<int>(x), static_cast<int>(y));
}

void GlfwWindow::GetWindowPos(unsigned int& x, unsigned int& y) {
	int xpos, ypos;
	glfwGetWindowPos(windowHandle, &xpos, &ypos);
	x = static_cast<unsigned int>(xpos);
	y = static_cast<unsigned int>(ypos);
}

void GlfwWindow::Close() {
	glfwSetWindowShouldClose(windowHandle, GLFW_TRUE);
}

GLFWwindow* GlfwWindow::GetHandle() {
	return windowHandle;
}

void GlfwWindow::SetWindowFocus(bool isFocused) {
	if (windowHandle) {
		glfwFocusWindow(windowHandle);
	}
}

bool GlfwWindow::GetWindowFocus() {
	return glfwGetWindowAttrib(windowHandle, GLFW_FOCUSED) == GLFW_TRUE;
}

bool GlfwWindow::GetWindowMinimized() {
	return glfwGetWindowAttrib(windowHandle, GLFW_ICONIFIED) == GLFW_TRUE;
}

void GlfwWindow::GetTitle(char* allocatedBuffer) {
	// TODO: Implement this
}

void GlfwWindow::SetTitle(const char* title) {
	glfwSetWindowTitle(windowHandle, title);
}

void GlfwWindow::SetWindowAlpha(float alpha) {
	glfwSetWindowOpacity(windowHandle, alpha);
}

float GlfwWindow::GetWindowDpiScale() {
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

	return Events::KeyPressCode::Invalid;
}
