#pragma once

#include <vector>
#include "Window.hpp"

namespace Grindstone {
	class WindowManager {
	public:
		virtual Window* Create(Window::CreateInfo&);
		virtual Window* GetWindowByIndex(unsigned int i);
		virtual unsigned int GetNumWindows();
		virtual void UpdateWindows();
	private:
        std::vector<Window*> windows;
	};
};
