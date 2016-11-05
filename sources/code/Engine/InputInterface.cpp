#include "InputInterface.h"
#include "Engine.h"

void InputInterface::ResizeEvent(int, int) {}
void InputInterface::SetMousePosition(int, int) {}
void InputInterface::SetMouseButton(int mb, bool state) {
    if (state)
	    std::cout << "Mouse Button pressed: " << mb << std::endl;
    else
	    std::cout << "Mouse Button released: " << mb << std::endl;
}

void InputInterface::SetMouseInWindow(bool) {}
bool InputInterface::IsMouseInWindow() {
	return true;
}
void InputInterface::SetFocused(bool) {}
bool InputInterface::IsFocused() {
	return true;
}

void InputInterface::SetKey(int key, bool state) {
    if (state)
    	std::cout << "Key pressed: " << key << std::endl;
    else
    	std::cout << "Key released: " << key << std::endl;

    if (key == KEY_ESCAPE)
        ForceQuit();
}

void InputInterface::Quit() {
    ForceQuit();
}

void InputInterface::ForceQuit() {
    engine.Shutdown();
}