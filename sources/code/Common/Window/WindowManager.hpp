#pragma once

#include <vector>
#include "Window.hpp"

class InputInterface;


namespace Grindstone {
	class WindowManager {
	public:
		virtual Window* createWindow(Window::CreateInfo&);
		virtual Window* getWindowByIndex(unsigned int i);
		virtual unsigned int getNumWindows();
		virtual void updateWindows();
	private:
        std::vector<Window*> windows;
	};
};