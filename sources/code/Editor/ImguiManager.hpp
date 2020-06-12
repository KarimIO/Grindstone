#ifndef _IMGUI_MANAGER_H
#define _IMGUI_MANAGER_H

#include "imgui/imgui.h"

class ImguiManager {
public:
    void initialize(BaseWindow* win);
    ~ImguiManager();

	IMGUI_IMPL_API void NewFrame();

	void UpdateMousePosAndButtons();
	void UpdateMouseCursor();

	// InitXXX function with 'install_callbacks=true': install GLFW callbacks. They will call user's previously installed callbacks, if any.
	// InitXXX function with 'install_callbacks=false': do not install GLFW callbacks. You will need to call them yourself from your own GLFW callbacks.
	IMGUI_IMPL_API void MouseButtonCallback(int button, int action, int mods);
	IMGUI_IMPL_API void ScrollCallback(double xoffset, double yoffset);
	IMGUI_IMPL_API void KeyCallback(int key, int action);
	IMGUI_IMPL_API void CharCallback(unsigned int c);

private:
	void Init(bool install_callbacks);
	IMGUI_IMPL_API void Shutdown();
};

#endif