#include <imgui.h>
#include "Common/Event/BaseEvent.hpp"
#include "Common/Event/KeyEvent.hpp"
#include "Common/Event/MouseEvent.hpp"
#include "Common/Event/KeyPressCode.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Events/Dispatcher.hpp"
#include "ImguiInput.hpp"
using namespace std::placeholders;
using namespace Grindstone::Editor::ImguiEditor;
using namespace Grindstone::Events;

bool ImguiInput::OnMouseMove(BaseEvent* ev) {
	MouseMovedEvent* evCast = (MouseMovedEvent*)ev;
	io.MousePos.x = (float)evCast->mouseX;
	io.MousePos.y = (float)evCast->mouseY;
	return true;
}

bool ImguiInput::OnMousePressed(BaseEvent* ev) {
	MousePressEvent* evCast = (MousePressEvent*)ev;
	int mouseBtn = (int)evCast->code;
	if (mouseBtn == 2) {
		mouseBtn = 1;
	}
	else if (mouseBtn == 1) {
		mouseBtn = 2;
	}

	io.MouseDown[mouseBtn] = evCast->isPressed;
	return true;
}

bool ImguiInput::OnMouseScrolled(BaseEvent* ev) {
	MouseScrolledEvent* evCast = (MouseScrolledEvent*)ev;
	io.MouseWheel = evCast->scrollY;
	io.MouseWheelH = evCast->scrollX;
	return true;
}

bool ImguiInput::OnKeyPressed(BaseEvent* ev) {
	KeyPressEvent* evCast = (KeyPressEvent*)ev;
	io.KeysDown[(int)evCast->code] = evCast->isPressed;

	io.KeyCtrl = io.KeysDown[(int)KeyPressCode::LeftControl] || io.KeysDown[(int)KeyPressCode::Control];
	io.KeyShift = io.KeysDown[(int)KeyPressCode::LeftShift] || io.KeysDown[(int)KeyPressCode::Shift];
	io.KeyAlt = io.KeysDown[(int)KeyPressCode::LeftAlt] || io.KeysDown[(int)KeyPressCode::Alt];
	// io.KeySuper = io.KeysDown[(int)KeyPressCode:] || io.KeysDown[(int)KeyPressCode::RIGHT_SUPER];

	return true;
}

bool ImguiInput::OnCharacterTyped(BaseEvent* ev) {
	CharacterTypedEvent* evCast = (CharacterTypedEvent*)ev;
	io.AddInputCharacter(evCast->character);
	return true;
}

ImguiInput::ImguiInput(ImGuiIO& io, EngineCore* engineCore) : io(io) {
	auto eventDispatcher = engineCore->GetEventDispatcher();

	eventDispatcher->AddEventListener(EventType::MouseMoved, std::bind(&ImguiInput::OnMouseMove, this, _1));
	eventDispatcher->AddEventListener(EventType::MouseButton, std::bind(&ImguiInput::OnMousePressed, this, _1));
	eventDispatcher->AddEventListener(EventType::MouseScrolled, std::bind(&ImguiInput::OnMouseScrolled, this, _1));
	eventDispatcher->AddEventListener(EventType::KeyPress, std::bind(&ImguiInput::OnKeyPressed, this, _1));
	eventDispatcher->AddEventListener(EventType::CharacterTyped, std::bind(&ImguiInput::OnCharacterTyped, this, _1));

	io.KeyMap[ImGuiKey_Tab] = (int)Events::KeyPressCode::Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = (int)Events::KeyPressCode::ArrowLeft;
	io.KeyMap[ImGuiKey_RightArrow] = (int)Events::KeyPressCode::ArrowRight;
	io.KeyMap[ImGuiKey_UpArrow] = (int)Events::KeyPressCode::ArrowUp;
	io.KeyMap[ImGuiKey_DownArrow] = (int)Events::KeyPressCode::ArrowDown;
	io.KeyMap[ImGuiKey_PageUp] = (int)Events::KeyPressCode::PageUp;
	io.KeyMap[ImGuiKey_PageDown] = (int)Events::KeyPressCode::PageDown;
	io.KeyMap[ImGuiKey_Home] = (int)Events::KeyPressCode::Home;
	io.KeyMap[ImGuiKey_End] = (int)Events::KeyPressCode::End;
	io.KeyMap[ImGuiKey_Insert] = (int)Events::KeyPressCode::Insert;
	io.KeyMap[ImGuiKey_Delete] = (int)Events::KeyPressCode::Delete;
	io.KeyMap[ImGuiKey_Backspace] = (int)Events::KeyPressCode::Backspace;
	io.KeyMap[ImGuiKey_Space] = (int)Events::KeyPressCode::Space;
	io.KeyMap[ImGuiKey_Enter] = (int)Events::KeyPressCode::Enter;
	io.KeyMap[ImGuiKey_Escape] = (int)Events::KeyPressCode::Escape;
	io.KeyMap[ImGuiKey_A] = (int)Events::KeyPressCode::A;
	io.KeyMap[ImGuiKey_C] = (int)Events::KeyPressCode::C;
	io.KeyMap[ImGuiKey_V] = (int)Events::KeyPressCode::V;
	io.KeyMap[ImGuiKey_X] = (int)Events::KeyPressCode::X;
	io.KeyMap[ImGuiKey_Y] = (int)Events::KeyPressCode::Y;
	io.KeyMap[ImGuiKey_Z] = (int)Events::KeyPressCode::Z;
}
