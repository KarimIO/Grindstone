#include "Window.hpp"
#include "Win32Window.hpp"

using namespace Grindstone;

Window* Window::create(CreateInfo& create_info) {
#ifdef _WIN32
	Win32Window *win = new Win32Window();
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
