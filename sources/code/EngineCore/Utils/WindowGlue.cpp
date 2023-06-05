#include <EngineCore/pch.hpp>
#include <Common/Window/Window.hpp>
#include <Common/Window/WindowManager.hpp>
#include <EngineCore/EngineCore.hpp>

using namespace Grindstone;

char windowTitleBuffer[256];

extern "C" {
	ENGINE_CORE_API Window* WindowGetCurrentWindow() {
		auto windowManager = EngineCore::GetInstance().windowManager;
		if (windowManager->GetNumWindows() == 0) {
			return nullptr;
		}

		return windowManager->GetWindowByIndex(0);
	}

	ENGINE_CORE_API void WindowClose(Window* window) {
		window->Close();
	}

	ENGINE_CORE_API void WindowShow(Window* window) {
		window->Show();
	}

	ENGINE_CORE_API void WindowHide(Window* window) {
		window->Hide();
	}

	ENGINE_CORE_API void WindowSetFullscreenMode(Window* window, Window::FullscreenMode value) {
		window->SetFullscreen(value);
	}

	ENGINE_CORE_API void WindowGetRect(Window* window, uint32_t& left, uint32_t& top, uint32_t& right, uint32_t& bottom) {
		window->GetWindowRect(left, top, right, bottom);
	}

	ENGINE_CORE_API void WindowGetSize(Window* window, uint32_t& width, uint32_t& height) {
		window->GetWindowSize(width, height);
	}

	ENGINE_CORE_API void WindowSetSize(Window* window, uint32_t width, uint32_t height) {
		window->SetWindowSize(width, height);
	}

	ENGINE_CORE_API void WindowGetPosition(Window* window, int& x, int& y) {
		// TODO: Should this be a uint or int?
		unsigned int winX, winY;
		window->GetWindowPos(winX, winY);
		x = winX;
		y = winY;
	}

	ENGINE_CORE_API void WindowSetPosition(Window* window, int x, int y) {
		window->SetMousePos(x, y);
	}

	ENGINE_CORE_API bool WindowGetIsFocused(Window* window) {
		return window->GetWindowFocus();
	}

	ENGINE_CORE_API void WindowSetIsFocused(Window* window, bool isFocused) {
		window->SetWindowFocus(isFocused);
	}

	ENGINE_CORE_API bool WindowGetIsMinimized(Window* window) {
		return window->GetWindowMinimized();
	}

	ENGINE_CORE_API const char* WindowGetTitle(Window* window) {
		window->GetTitle(windowTitleBuffer);
		return windowTitleBuffer;
	}

	ENGINE_CORE_API void WindowSetTitle(Window* window, const char* title) {
		window->SetTitle(title);
	}
}
