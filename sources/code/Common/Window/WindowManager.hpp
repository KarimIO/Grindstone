#pragma once

#include <vector>
#include "Window.hpp"

namespace Grindstone {
	class WindowManager {
	public:
		virtual Window* Create(Window::CreateInfo&);
		virtual Window* GetWindowByIndex(unsigned int i);
		virtual unsigned int GetNumWindows();
		virtual void CloseWindow(Grindstone::Window* window);
		virtual void UpdateWindows();
		virtual bool AreAllWindowsClosed();
	private:
        std::vector<Window*> windows;
	};
};
