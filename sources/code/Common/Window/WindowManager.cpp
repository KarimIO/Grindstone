#include "WindowManager.hpp"
#if defined(_WIN32)
	#include "Win32Window.hpp"
#elif defined(__linux__)
	#include "X11Window.hpp"
#endif

Grindstone::Window* Grindstone::WindowManager::Create(Grindstone::Window::CreateInfo& createInfo) {
#if defined(_WIN32)
	Grindstone::Win32Window *win = new Grindstone::Win32Window();
#elif defined(__linux__)
	Grindstone::X11Window *win = new Grindstone::X11Window();
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

Grindstone::Window* Grindstone::WindowManager::GetWindowByIndex(unsigned int i) {
	return windows[i];
}

unsigned int Grindstone::WindowManager::GetNumWindows() {
	return (unsigned int)windows.size();
}

void Grindstone::WindowManager::CloseWindow(Grindstone::Window* window) {
	window->Close();

	for (size_t i = 0; i < windows.size(); ++i) {
		if (windows[i] == window) {
			windows.erase(windows.begin() + i);
		}
	}
}

void Grindstone::WindowManager::UpdateWindows() {
	for (auto window : windows) {
		window->ImmediateSwapBuffers();
		window->HandleEvents();
	}
}

bool Grindstone::WindowManager::AreAllWindowsClosed() {
	return windows.empty();
}
