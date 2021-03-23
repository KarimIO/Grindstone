#include "InputManager.hpp"
#include <iostream>
using namespace Grindstone;

void Input::Manager::ResizeEvent(int, int) {
}

void Input::Manager::SetMouseButton(MouseAction, Input::ButtonStatus) {
std::cout << "klik\n";
}

Input::ButtonStatus Input::Manager::GetMouseButton(MouseAction) {
	return ButtonStatus();
}

void Input::Manager::SetMousePosition(int x, int y) {
}

void Input::Manager::GetMousePosition(int&, int&) {
}

void Input::Manager::SetFocused(bool) {
}

bool Input::Manager::IsFocused() {
	return false;
}

void Input::Manager::SetKey(KeyAction, Input::ButtonStatus) {
}

Input::ButtonStatus Input::Manager::GetKey(KeyAction) {
	return ButtonStatus();
}

void Input::Manager::Quit() {
}

void Input::Manager::ForceQuit() {
}
