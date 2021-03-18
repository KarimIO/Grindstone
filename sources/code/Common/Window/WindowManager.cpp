#include "WindowManager.hpp"
using namespace Grindstone;

#if defined(_WIN32)
	#include "Win32Window.hpp"
#elif defined(__linux__)
	#include "X11Window.hpp"
#endif

Window* WindowManager::createWindow(Window::CreateInfo& createInfo) {
#if defined(_WIN32)
	Grindstone::Win32Window *win = new Grindstone::Win32Window();
#elif defined(__linux__)
	Grindstone::X11Window *win = nullptr; //new Grindstone::X11Window();
#else
	return nullptr;
#endif

	if (win->initialize(createInfo)) {
		windows.push_back(win);
		return win;
	}
	else {
		delete win;
		return nullptr;
	}
}

Window* WindowManager::getWindowByIndex(unsigned int i) {
	return windows[i];
}

unsigned int WindowManager::getNumWindows() {
	return windows.size();
}

void WindowManager::updateWindows() {
	for (auto window : windows) {
		window->immediateSwapBuffers();
		window->handleEvents();
	}
}
