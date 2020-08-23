#include "Window.hpp"

#if defined(_WIN32)
	#include "Win32Window.hpp"
#elif defined(__linux__)
	#include "X11Window.hpp"
#endif

Grindstone::Window* Grindstone::Window::create(CreateInfo& create_info) {
#if defined(_WIN32)
	Grindstone::Win32Window *win = new Grindstone::Win32Window();
#elif defined(__linux__)
	Grindstone::X11Window *win = nullptr; //new Grindstone::X11Window();
#else
	return nullptr;
#endif

	if (win->initialize(create_info))
		return win;
	else {
		delete win;
		return nullptr;
	}
}
