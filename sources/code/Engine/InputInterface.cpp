#include "InputInterface.h"
#include "Engine.h"

void InputInterface::ResizeEvent(int, int) {}
void InputInterface::SetMousePosition(int x, int y) {
	float sensitivity = 0.01f;
	float scale = 768 / 2 - y;
	engine.angles.x += sensitivity * scale;

	if (engine.angles.x < -2.4f / 2)	engine.angles.x = -2.4f / 2;
	if (engine.angles.x > 3.14f / 2)	engine.angles.x = 3.14f / 2;
	scale = 1024 / 2 - x;

	engine.angles.y += sensitivity * scale;
}

void InputInterface::SetMouseButton(int mb, bool state) {
    if (state)
	    std::cout << "Mouse Button pressed: " << mb << "\n";
    else
	    std::cout << "Mouse Button released: " << mb << "\n";
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
	float scale = 0.5;

	if (key == KEY_W)
		engine.position += scale * engine.getForward();
	if (key == KEY_S)
		engine.position -= scale * engine.getForward();

	if (key == KEY_A)
		engine.position -= scale * engine.getRight();
	if (key == KEY_D)
		engine.position += scale * engine.getRight();

	if (key == KEY_SPACE)
		engine.position += scale * engine.getUp();
	if (key == KEY_CONTROL)
		engine.position -= scale * engine.getUp();

    if (key == KEY_ESCAPE)
        ForceQuit();
}

void InputInterface::Quit() {
    ForceQuit();
}

void InputInterface::ForceQuit() {
    engine.Shutdown();
}