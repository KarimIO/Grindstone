#include "InputSystem.hpp"
#include <iostream>
using namespace Grindstone;

void Input::Manager::resizeEvent(int, int) {
}

void Input::Manager::setMouseButton(MouseAction, Input::ButtonStatus) {
}

Input::ButtonStatus Input::Manager::getMouseButton(MouseAction) {
	return ButtonStatus();
}

void Input::Manager::setMousePosition(int x, int y) {
}

void Input::Manager::getMousePosition(int&, int&) {
}

void Input::Manager::setFocused(bool) {
}

bool Input::Manager::isFocused() {
	return false;
}

void Input::Manager::setKey(KeyAction, Input::ButtonStatus) {
}

Input::ButtonStatus Input::Manager::getKey(KeyAction) {
	return ButtonStatus();
}

void Input::Manager::quit() {
}

void Input::Manager::forceQuit() {
}
