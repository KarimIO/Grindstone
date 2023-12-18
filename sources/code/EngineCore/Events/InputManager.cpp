#include <iostream>

#include <EngineCore/Events/Dispatcher.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Event/MouseEvent.hpp>
#include <Common/Event/KeyEvent.hpp>
#include <Common/Event/WindowEvent.hpp>
#include <Common/Math.hpp>
#include <Common/Input/CursorMode.hpp>
#include <Common/Window/Window.hpp>

#include "InputManager.hpp"

using namespace Grindstone::Input;
using namespace Grindstone::Events;

extern "C" {
	ENGINE_CORE_API void InputManagerSetMouseIsRawMotion(bool isRawMode) {
		Grindstone::EngineCore::GetInstance().GetInputManager()->SetCursorIsRawMotion(isRawMode);
	}

	ENGINE_CORE_API bool InputManagerGetMouseIsRawMotion() {
		return Grindstone::EngineCore::GetInstance().GetInputManager()->GetCursorIsRawMotion();
	}

	ENGINE_CORE_API void InputManagerSetCursorMode(uint8_t mode) {
		Grindstone::EngineCore::GetInstance().GetInputManager()->SetCursorMode(static_cast<Grindstone::Input::CursorMode>(mode));
	}

	ENGINE_CORE_API uint8_t InputManagerGetCursorMode() {
		return static_cast<uint8_t>(Grindstone::EngineCore::GetInstance().GetInputManager()->GetCursorMode());
	}

	ENGINE_CORE_API bool InputManagerGetIsWindowFocused() {
		return Grindstone::EngineCore::GetInstance().GetInputManager()->IsFocused();
	}

	ENGINE_CORE_API bool InputManagerIsKeyDown(int keyboardKey) {
		return Grindstone::EngineCore::GetInstance().GetInputManager()->IsKeyPressed((KeyPressCode)keyboardKey);
	}

	ENGINE_CORE_API bool InputManagerIsMouseButtonDown(int mouseButton) {
		return Grindstone::EngineCore::GetInstance().GetInputManager()->IsMouseButtonPressed((MouseButtonCode)mouseButton);
	}

	ENGINE_CORE_API void InputManagerSetMousePos(Grindstone::Math::Float2 mousePos) {
		int mpx = (int)mousePos[0];
		int mpy = (int)mousePos[1];
		Grindstone::EngineCore::GetInstance().GetInputManager()->SetMousePosition((int)mpx, (int)mpy);
	}

	struct Float2 {
		float x, y;
	};

	Float2 savedMousePos;
	ENGINE_CORE_API Float2 InputManagerGetMousePos() {
		int x, y;
		Grindstone::EngineCore::GetInstance().GetInputManager()->GetMousePosition(x, y);
		savedMousePos.x = static_cast<float>(x);
		savedMousePos.y = static_cast<float>(y);

		return savedMousePos;
	}
}

Manager::Manager(Events::Dispatcher* dispatcher) : dispatcher(dispatcher) {
	std::memset(keyPressed, 0, sizeof(keyPressed));
	std::memset(mousePressed, 0, sizeof(mousePressed));
}

void Manager::SetCursorMode(CursorMode cursorMode) {
	window->SetCursorMode(cursorMode);
}

CursorMode Manager::GetCursorMode() {
	return window->GetCursorMode();
}

void Manager::SetCursorIsRawMotion(bool isRawMode) {
	window->SetMouseIsRawMotion(isRawMode);
}

bool Manager::GetCursorIsRawMotion() {
	return window->GetMouseIsRawMotion();
}

void Manager::SetMainWindow(Grindstone::Window* window) {
	this->window = window;
}

void Manager::ResizeEvent(int width, int height) {
	WindowResizeEvent* ev = new WindowResizeEvent{ width, height };
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

void Manager::OnMouseMoved(int x, int y) {
	MouseMovedEvent* ev = new MouseMovedEvent{x, y};
	mousePositionX = x;
	mousePositionY = y;
	dispatcher->Dispatch(ev);
}

void Manager::SetMousePosition(int x, int y) {
	window->SetMousePos(x, y);
}

void Manager::GetMousePosition(int&x, int&y) {
	unsigned int ux, uy;
	window->GetMousePos(ux, uy);
	x = ux;
	y = uy;
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

void Manager::TryQuit(Grindstone::Window* window) {
	WindowTryQuitEvent* ev = new WindowTryQuitEvent(window);
	dispatcher->Dispatch(ev);
}

void Manager::ForceQuit(Grindstone::Window* window) {
	WindowForceQuitEvent* ev = new WindowForceQuitEvent(window);
	dispatcher->Dispatch(ev);
}
