#pragma once

#include <vector>
#include "Window.hpp"

namespace Grindstone {
	class WindowManager {
	public:
		~WindowManager();

		virtual Grindstone::Window* Create(Grindstone::Window::CreateInfo&);
		virtual Grindstone::Window* GetWindowByIndex(unsigned int i);
		virtual unsigned int GetNumWindows();
		virtual void CloseWindow(Grindstone::Window* window);
		virtual void UpdateWindows();
		virtual bool AreAllWindowsClosed();
	private:
        std::vector<Window*> windows;
	};
};
