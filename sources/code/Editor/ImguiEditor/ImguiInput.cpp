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
	io.AddMousePosEvent((float)evCast->mouseX, (float)evCast->mouseY);
	return false;
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

	io.AddMouseButtonEvent(mouseBtn, evCast->isPressed);
	return false;
}

bool ImguiInput::OnMouseScrolled(BaseEvent* ev) {
	MouseScrolledEvent* evCast = (MouseScrolledEvent*)ev;
	io.AddMouseWheelEvent(evCast->scrollX, evCast->scrollY);
	return false;
}

static ImGuiKey TranslateKeycodeToImgui(Grindstone::Events::KeyPressCode keycode) {
	switch (keycode) {
		case Grindstone::Events::KeyPressCode::Tab: return ImGuiKey_Tab;
		case Grindstone::Events::KeyPressCode::ArrowLeft: return ImGuiKey_LeftArrow;
		case Grindstone::Events::KeyPressCode::ArrowRight: return ImGuiKey_RightArrow;
		case Grindstone::Events::KeyPressCode::ArrowUp: return ImGuiKey_UpArrow;
		case Grindstone::Events::KeyPressCode::ArrowDown: return ImGuiKey_DownArrow;
		case Grindstone::Events::KeyPressCode::PageUp: return ImGuiKey_PageUp;
		case Grindstone::Events::KeyPressCode::PageDown: return ImGuiKey_PageDown;
		case Grindstone::Events::KeyPressCode::Home: return ImGuiKey_Home;
		case Grindstone::Events::KeyPressCode::End: return ImGuiKey_End;
		case Grindstone::Events::KeyPressCode::Insert: return ImGuiKey_Insert;
		case Grindstone::Events::KeyPressCode::Delete: return ImGuiKey_Delete;
		case Grindstone::Events::KeyPressCode::Backspace: return ImGuiKey_Backspace;
		case Grindstone::Events::KeyPressCode::Space: return ImGuiKey_Space;
		case Grindstone::Events::KeyPressCode::Enter: return ImGuiKey_Enter;
		case Grindstone::Events::KeyPressCode::Escape: return ImGuiKey_Escape;
		case Grindstone::Events::KeyPressCode::A: return ImGuiKey_A;
		case Grindstone::Events::KeyPressCode::C: return ImGuiKey_C;
		case Grindstone::Events::KeyPressCode::V: return ImGuiKey_V;
		case Grindstone::Events::KeyPressCode::X: return ImGuiKey_X;
		case Grindstone::Events::KeyPressCode::Y: return ImGuiKey_Y;
		case Grindstone::Events::KeyPressCode::Z: return ImGuiKey_Z;
	}

	return ImGuiKey_None;
}

bool ImguiInput::OnKeyPressed(BaseEvent* ev) {
	KeyPressEvent* evCast = (KeyPressEvent*)ev;

	ImGuiKey keycode = TranslateKeycodeToImgui(evCast->code);
	if (keycode == ImGuiKey_None) {
		return false;
	}

	io.AddKeyEvent(keycode, evCast->isPressed);

	// io.KeyCtrl = io.KeysData[(int)KeyPressCode::LeftControl] || io.KeysData[(int)KeyPressCode::Control];
	// io.KeyShift = io.KeysData[(int)KeyPressCode::LeftShift] || io.KeysData[(int)KeyPressCode::Shift];
	// io.KeyAlt = io.KeysData[(int)KeyPressCode::LeftAlt] || io.KeysData[(int)KeyPressCode::Alt];
	// io.KeySuper = io.KeysDown[(int)KeyPressCode:] || io.KeysDown[(int)KeyPressCode::RIGHT_SUPER];

	return false;
}

bool ImguiInput::OnCharacterTyped(BaseEvent* ev) {
	CharacterTypedEvent* evCast = (CharacterTypedEvent*)ev;
	io.AddInputCharacter(evCast->character);
	return false;
}

ImguiInput::ImguiInput(ImGuiIO& io, EngineCore* engineCore) : io(io) {
	auto eventDispatcher = engineCore->GetEventDispatcher();

	eventDispatcher->AddEventListener(EventType::MouseMoved, std::bind(&ImguiInput::OnMouseMove, this, _1));
	eventDispatcher->AddEventListener(EventType::MouseButton, std::bind(&ImguiInput::OnMousePressed, this, _1));
	eventDispatcher->AddEventListener(EventType::MouseScrolled, std::bind(&ImguiInput::OnMouseScrolled, this, _1));
	eventDispatcher->AddEventListener(EventType::KeyPress, std::bind(&ImguiInput::OnKeyPressed, this, _1));
	eventDispatcher->AddEventListener(EventType::CharacterTyped, std::bind(&ImguiInput::OnCharacterTyped, this, _1));
}
