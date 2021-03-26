#include "InputManager.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "Common/Event/MouseEvent.hpp"
#include "Common/Event/KeyEvent.hpp"
#include "Common/Event/WindowEvent.hpp"
using namespace Grindstone::Input;
using namespace Grindstone::Events;

Manager::Manager(Events::Dispatcher* dispatcher) : dispatcher(dispatcher) {}

void Manager::ResizeEvent(int width, int height) {
	WindowResizeEvent* ev = new WindowResizeEvent{width, height};
	dispatcher->Dispatch(ev);
}

void Manager::SetMouseButton(Events::MouseButtonCode code, bool isPressed) {
	mousePressed[(int)code] = isPressed;

	MousePressEvent* ev = new MousePressEvent{code, isPressed};
	dispatcher->Dispatch(ev);
}

bool Manager::IsMouseButtonPressed(Events::MouseButtonCode code) {
	return mousePressed[(int)code];
}

void Manager::SetMousePosition(int x, int y) {
	MouseMovedEvent* ev = new MouseMovedEvent{x, y};
	mousePositionX = x;
	mousePositionY = y;
	dispatcher->Dispatch(ev);
}

void Manager::GetMousePosition(int&x, int&y) {
	x = mousePositionX;
	y = mousePositionY;
}

void Manager::SetIsFocused(bool isFocused) {
	this->isFocused = isFocused;

	BaseEvent* ev;
	if (isFocused) {
		ev = new WindowStartFocusEvent{};
	}
	else {
		ev = new WindowKillFocusEvent{};
	}
	dispatcher->Dispatch(ev);
}

bool Manager::IsFocused() {
	return isFocused;
}

void Manager::MouseScroll(float offsetX, float offsetY) {
	MouseScrolledEvent* ev = new MouseScrolledEvent{offsetX, offsetY};
	dispatcher->Dispatch(ev);
}

void Manager::SetKeyPressed(Events::KeyPressCode code, bool isPressed) {
	KeyPressEvent* ev = new KeyPressEvent{ code, isPressed };
	keyPressed[(int)code] = isPressed;
	dispatcher->Dispatch(ev);
}

void Manager::AddCharacterTyped(unsigned short character) {
	CharacterTypedEvent* ev = new CharacterTypedEvent{ character };
	dispatcher->Dispatch(ev);
}

bool Manager::IsKeyPressed(Events::KeyPressCode code) {
	return keyPressed[(int)code];
}

void Manager::Quit() {
	WindowCloseEvent* ev = new WindowCloseEvent();
	dispatcher->Dispatch(ev);
}

void Manager::ForceQuit() {

}

