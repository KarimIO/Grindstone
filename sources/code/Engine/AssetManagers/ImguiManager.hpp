#ifndef _IMGUI_MANAGER_H
#define _IMGUI_MANAGER_H

#include <dear_imgui/imgui.h>
#include <WindowModule/BaseWindow.hpp>
#include <Engine/Core/InputInterface.hpp>

class ImguiManager : public InputInterface {
public:
    void initialize(BaseWindow* window_);
    ~ImguiManager();

	void newFrame(double time);

	void UpdateMousePos();
	void UpdateMouseCursor();

	// InitXXX function with 'install_callbacks=true': install GLFW callbacks. They will call user's previously installed callbacks, if any.
	// InitXXX function with 'install_callbacks=false': do not install GLFW callbacks. You will need to call them yourself from your own GLFW callbacks.
	/*IMGUI_IMPL_API void MouseButtonCallback(int button, int action, int mods);
	IMGUI_IMPL_API void ScrollCallback(double xoffset, double yoffset);
	IMGUI_IMPL_API void KeyCallback(int key, int action);
	IMGUI_IMPL_API void CharCallback(unsigned int c);*/

	BaseWindow* getWindow();

	virtual void ResizeEvent(int, int);
	virtual void SetMouseButton(int, bool);
	virtual void SetMousePosition(int, int);
	virtual void SetFocused(bool);
	virtual bool IsFocused();
	virtual void SetKey(int, bool);
	virtual void Quit();
	virtual void ForceQuit();
	void swapBuffers();

private:
	IMGUI_IMPL_API void Shutdown();
private:
	BaseWindow *window_;
};

#endif