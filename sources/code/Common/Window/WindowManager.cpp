#include "WindowManager.hpp"
using namespace Grindstone;

#if defined(_WIN32)
	#include "Win32Window.hpp"
#elif defined(__linux__)
	#include "X11Window.hpp"
#endif

Window* WindowManager::Create(Window::CreateInfo& createInfo) {
#if defined(_WIN32)
	Grindstone::Win32Window *win = new Grindstone::Win32Window();
#elif defined(__linux__)
	Grindstone::X11Window *win = nullptr; //new Grindstone::X11Window();
#else
	return nullptr;
#endif

	if (win->Initialize(createInfo)) {
		windows.push_back(win);
		return win;
	}
	else {
		delete win;
		return nullptr;
	}
}

Window* WindowManager::GetWindowByIndex(unsigned int i) {
	return windows[i];
}

unsigned int WindowManager::GetNumWindows() {
	return (unsigned int)windows.size();
}

void WindowManager::UpdateWindows() {
	for (auto window : windows) {
		window->ImmediateSwapBuffers();
		window->HandleEvents();
	}
}
