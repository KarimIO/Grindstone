#include "InputManager.hpp"
#include "Common/Event/MouseEvent.hpp"
#include "Common/Event/KeyEvent.hpp"
#include "Common/Event/WindowEvent.hpp"
using namespace Grindstone::Input;
using namespace Grindstone::Events;

void Manager::ResizeEvent(int width, int height) {
	WindowResizeEvent* event = new WindowResizeEvent{width, height};
}

void Manager::SetMouseButton(Events::MouseButtonCode button, bool isPressed) {
	MousePressEvent* event = new MousePressEvent{button, isPressed};
}

bool Manager::IsMouseButtonPressed(Events::MouseButtonCode button) {
	return false;
}

void Manager::SetMousePosition(int x, int y) {
	MouseMovedEvent* event = new MouseMovedEvent{x, y};
}

void Manager::GetMousePosition(int&, int&) {

}

void Manager::SetIsFocused(bool isFocused) {
	if (isFocused) {
		WindowStartFocusEvent* event = new WindowStartFocusEvent{};
	}
	else {
		WindowKillFocusEvent* event = new WindowKillFocusEvent{};
	}
}

bool Manager::IsFocused() {
	return false;
}

void Manager::MouseScroll(int offsetX, int offsetY) {
	MouseScrolledEvent* event = new MouseScrolledEvent{offsetX, offsetY};

}

void Manager::SetKeyPressed(Events::KeyPressCode button, bool isPressed) {
	KeyPressEvent* event = new KeyPressEvent{button, isPressed};
}

bool Manager::IsKeyPressed(Events::KeyPressCode) {
	return false;
}

void Manager::Quit() {
	WindowCloseEvent* event = new WindowCloseEvent();
}

void Manager::ForceQuit() {

}

