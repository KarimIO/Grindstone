#if 0
#include "Input.h"

// Simply sets the modifier to 1.0f so that the default speed is set.
InputSystem::InputSystem() {
}

void InputSystem::ReadFile(std::string path) {
	CFile input(path);

	if (input.fail()) {
		std::cout << "Failed to load " << path << "\n";
		return;
	}

	while (!input.null()) {
		std::string command;
		std::string control;
		int controlID;
		float data;

		command = strToLower(input.streamStr());
		control = input.streamStr();
		data = input.streamFloat();
		controlID = GetMouseIDByName(control);

		if (controlID != -1)
			AddCommand(command.c_str(), mousePress, controlID, data);
		else {
			controlID = GetKeyIDByName(control);
			if (controlID != -1)
				AddCommand(command.c_str(), keyPress, controlID, data);
		}
	}
}

void InputSystem::AddCommand(const char *str, ControlContainer **ctrl, int i, float data) {
	ControlContainer *newCtrl = ctrl[i];
	if (newCtrl == nullptr) {
		ctrl[i] = new ControlContainer(str, nullptr);
		ctrl[i]->data = data;
	}
	else {
		do {
			newCtrl = newCtrl->next;
		} while (newCtrl->next != nullptr);

		newCtrl = new ControlContainer(str, nullptr);
		newCtrl->data = data;
	}
}

void InputSystem::LinkCommand(BaseControl *ctrl) {
	std::string command = strToLower(ctrl->control);
	for (size_t i = 0; i < KEY_LAST; i++) {
		ControlContainer *container = keyPress[i];
		while (container != nullptr) {
			if (container->command == command)
				container->control = ctrl;

			container = container->next;
		}
	}

	for (size_t i = 0; i < MOUSE_LAST; i++) {
		ControlContainer *container = mousePress[i];
		while (container != nullptr) {
			if (container->command == command) {
				container->control = ctrl;
			}

			container = container->next;
		}
	}
}

int InputSystem::GetMouseIDByName(std::string Control) {
	std::string control = strToLower(Control);
	if (control == "mouseleft")		return MOUSE_LEFT;
	if (control == "mousemiddle")	return MOUSE_MIDDLE;
	if (control == "mouseright")	return MOUSE_RIGHT;

	if (control == "mousewheelu")	return MOUSE_WHEEL_UP;
	if (control == "mousewheeld")	return MOUSE_WHEEL_DOWN;
	if (control == "mousewheell")	return MOUSE_WHEEL_LEFT;
	if (control == "mousewheelr")	return MOUSE_WHEEL_RIGHT;

	if (control == "mouse4")		return MOUSE_MOUSE4;
	if (control == "mouse4")		return MOUSE_MOUSE5;

	if (control == "mousex")		return MOUSE_XCOORD;
	if (control == "mousey")		return MOUSE_YCOORD;
	return -1;
}

void InputSystem::CreateNewComponent(std::string cfgFilePath, EBaseEntity *entity) {
	componentList.push_back(new InputComponent(cfgFilePath, this, entity, this));
}

int InputSystem::GetKeyIDByName(std::string Control) {
	std::string control = strToLower(Control);
	if (control == "escape")	return KEY_ESCAPE;
	if (control == "esc")		return KEY_ESCAPE;
	if (control == "tab")		return KEY_TAB;
	if (control == "space")		return KEY_SPACE;
	if (control == "lctrl")		return KEY_LCONTROL;
	if (control == "rctrl") 	return KEY_CONTROL;
	if (control == "lshift")	return KEY_LSHIFT;
	if (control == "rshift")	return KEY_SHIFT;
	if (control == "lalt")		return KEY_LALT;
	if (control == "ralt")		return KEY_ALT;
	if (control == "left")		return KEY_LEFT;
	if (control == "right")		return KEY_RIGHT;
	if (control == "up")		return KEY_UP;
	if (control == "down")		return KEY_DOWN;

	if (control == "a") return KEY_A;
	if (control == "b") return KEY_B;
	if (control == "c") return KEY_C;
	if (control == "d") return KEY_D;
	if (control == "e") return KEY_E;
	if (control == "f") return KEY_F;
	if (control == "g") return KEY_G;
	if (control == "h") return KEY_H;
	if (control == "i") return KEY_I;
	if (control == "j") return KEY_J;
	if (control == "k") return KEY_K;
	if (control == "l") return KEY_L;
	if (control == "m") return KEY_M;
	if (control == "n") return KEY_N;
	if (control == "o") return KEY_O;
	if (control == "p") return KEY_P;
	if (control == "q") return KEY_Q;
	if (control == "r") return KEY_R;
	if (control == "s") return KEY_S;
	if (control == "t") return KEY_T;
	if (control == "u") return KEY_U;
	if (control == "v") return KEY_V;
	if (control == "w") return KEY_W;
	if (control == "x") return KEY_X;
	if (control == "y") return KEY_Y;
	if (control == "z") return KEY_Z;

	if (control == "num0") return KEY_NUMPAD_0;
	if (control == "num1") return KEY_NUMPAD_1;
	if (control == "num2") return KEY_NUMPAD_2;
	if (control == "num3") return KEY_NUMPAD_3;
	if (control == "num4") return KEY_NUMPAD_4;
	if (control == "num5") return KEY_NUMPAD_5;
	if (control == "num6") return KEY_NUMPAD_6;
	if (control == "num7") return KEY_NUMPAD_7;
	if (control == "num8") return KEY_NUMPAD_8;
	if (control == "num9") return KEY_NUMPAD_9;

	if (control == "numlock")	return KEY_NUMPAD_NUMLOCK;
	if (control == "numdivide")	return KEY_NUMPAD_DIVIDE;
	if (control == "nummultiply")	return KEY_NUMPAD_MULTIPLY;
	if (control == "numsubtract")	return KEY_NUMPAD_SUBTRACT;
	if (control == "numadd")	return KEY_NUMPAD_ADD;
	if (control == "numenter")	return KEY_NUMPAD_ENTER;
	if (control == "numdot")	return KEY_NUMPAD_DOT;

	if (control == "f0")  return KEY_F0;
	if (control == "f1")  return KEY_F1;
	if (control == "f2")  return KEY_F2;
	if (control == "f3")  return KEY_F3;
	if (control == "f4")  return KEY_F4;
	if (control == "f5")  return KEY_F5;
	if (control == "f6")  return KEY_F6;
	if (control == "f7")  return KEY_F7;
	if (control == "f8")  return KEY_F8;
	if (control == "f9")  return KEY_F9;
	if (control == "f10") return KEY_F10;
	if (control == "f11") return KEY_F11;
	if (control == "f12") return KEY_F12;
	if (control == "f13") return KEY_F13;
	if (control == "f14") return KEY_F14;
	if (control == "f15") return KEY_F15;
	if (control == "f16") return KEY_F16;
	if (control == "f17") return KEY_F17;
	if (control == "f18") return KEY_F18;
	if (control == "f19") return KEY_F19;
	if (control == "f20") return KEY_F20;
	if (control == "f21") return KEY_F21;
	if (control == "f22") return KEY_F22;
	if (control == "f23") return KEY_F23;
	if (control == "f24") return KEY_F24;
	if (control == "f25") return KEY_F25;

	if (control == "0")	return KEY_0;
	if (control == "1")	return KEY_1;
	if (control == "2")	return KEY_2;
	if (control == "3")	return KEY_3;
	if (control == "4")	return KEY_4;
	if (control == "5")	return KEY_5;
	if (control == "6")	return KEY_6;
	if (control == "7")	return KEY_7;
	if (control == "8")	return KEY_8;
	if (control == "9")	return KEY_9;
	if (control == "dash")	return KEY_DASH;
	if (control == "add")	return KEY_ADD;

	if (control == "insert")	return KEY_INSERT;
	if (control == "home")		return KEY_HOME;
	if (control == "pgup")		return KEY_PG_UP;
	if (control == "pgdn")		return KEY_PG_DOWN;
	if (control == "end")		return KEY_END;
	if (control == "delete")	return KEY_DELETE;
	if (control == "pause")		return KEY_PAUSE;
	if (control == "capslock")	return KEY_CAPSLOCK;
	if (control == "scrolllock")	return KEY_SCROLL_LOCK;

	if (control == "comma")		return KEY_COMMA;
	if (control == "period")	return KEY_PERIOD;
	if (control == "fslash")	return KEY_FORWARD_SLASH;
	if (control == "slash")		return KEY_BACK_SLASH;
	if (control == "semicolon")	return KEY_SEMICOLON;
	if (control == "apostrophe")	return KEY_APOSTROPHE;
	if (control == "lbracket")	return KEY_LBRACKET;
	if (control == "rbracket")	return KEY_RBRACKET;

	if (control == "enter")		return KEY_ENTER;
	if (control == "backspace")	return KEY_BACKSPACE;
	if (control == "tilde")		return KEY_TILDE;

	return -1;
}

std::string InputSystem::getKeyString(int key) {
	switch (key) {
	case KEY_ESCAPE: return "esc";
	case KEY_TAB: return "tab";
	case KEY_SPACE: return "space";
	case KEY_LCONTROL: return "lctrl";
	case KEY_CONTROL: return "rctrl";
	case KEY_LSHIFT: return "lshift";
	case KEY_SHIFT: return "rshift";
	case KEY_LALT: return "lalt";
	case KEY_ALT: return "ralt";
	case KEY_LEFT: return "left";
	case KEY_RIGHT: return "right";
	case KEY_UP: return "up";
	case KEY_DOWN: return "down";

	case KEY_A: return "a";
	case KEY_B: return "b";
	case KEY_C: return "c";
	case KEY_D: return "d";
	case KEY_E: return "e";
	case KEY_F: return "f";
	case KEY_G: return "g";
	case KEY_H: return "h";
	case KEY_I: return "i";
	case KEY_J: return "j";
	case KEY_K: return "k";
	case KEY_L: return "l";
	case KEY_M: return "m";
	case KEY_N: return "n";
	case KEY_O: return "o";
	case KEY_P: return "p";
	case KEY_Q: return "q";
	case KEY_R: return "r";
	case KEY_S: return "s";
	case KEY_T: return "t";
	case KEY_U: return "u";
	case KEY_V: return "v";
	case KEY_W: return "w";
	case KEY_X: return "x";
	case KEY_Y: return "y";
	case KEY_Z: return "z";

	case KEY_NUMPAD_0: return "num0";
	case KEY_NUMPAD_1: return "num1";
	case KEY_NUMPAD_2: return "num2";
	case KEY_NUMPAD_3: return "num3";
	case KEY_NUMPAD_4: return "num4";
	case KEY_NUMPAD_5: return "num5";
	case KEY_NUMPAD_6: return "num6";
	case KEY_NUMPAD_7: return "num7";
	case KEY_NUMPAD_8: return "num8";
	case KEY_NUMPAD_9: return "num9";

	case KEY_NUMPAD_NUMLOCK: return "numlock";
	case KEY_NUMPAD_DIVIDE: return "numdivide";
	case KEY_NUMPAD_MULTIPLY: return "nummultiply";
	case KEY_NUMPAD_SUBTRACT: return "numsubtract";
	case KEY_NUMPAD_ADD: return "numadd";
	case KEY_NUMPAD_ENTER: return "numenter";
	case KEY_NUMPAD_DOT: return "numdot";

	case KEY_F0: return "f0";
	case KEY_F1: return "f1";
	case KEY_F2: return "f2";
	case KEY_F3: return "f3";
	case KEY_F4: return "f4";
	case KEY_F5: return "f5";
	case KEY_F6: return "f6";
	case KEY_F7: return "f7";
	case KEY_F8: return "f8";
	case KEY_F9: return "f9";
	case KEY_F10: return "f10";
	case KEY_F11: return "f11";
	case KEY_F12: return "f12";
	case KEY_F13: return "f13";
	case KEY_F14: return "f14";
	case KEY_F15: return "f15";
	case KEY_F16: return "f16";
	case KEY_F17: return "f17";
	case KEY_F18: return "f18";
	case KEY_F19: return "f19";
	case KEY_F20: return "f20";
	case KEY_F21: return "f21";
	case KEY_F22: return "f22";
	case KEY_F23: return "f23";
	case KEY_F24: return "f24";
	case KEY_F25: return "f25";

	case KEY_0: return "0";
	case KEY_1: return "1";
	case KEY_2: return "2";
	case KEY_3: return "3";
	case KEY_4: return "4";
	case KEY_5: return "5";
	case KEY_6: return "6";
	case KEY_7: return "7";
	case KEY_8: return "8";
	case KEY_9: return "9";
	case KEY_DASH: return "dash";
	case KEY_ADD: return "add";

	case KEY_INSERT: return "insert";
	case KEY_HOME: return "home";
	case KEY_PG_UP: return "pgup";
	case KEY_PG_DOWN: return "pgdn";
	case KEY_END: return "end";
	case KEY_DELETE: return "delete";
	case KEY_PAUSE: return "pause";
	case KEY_CAPSLOCK: return "capslock";
	case KEY_SCROLL_LOCK: return "scrolllock";

	case KEY_COMMA: return "comma";
	case KEY_PERIOD: return "period";
	case KEY_FORWARD_SLASH: return "fslash";
	case KEY_BACK_SLASH: return "slash";
	case KEY_SEMICOLON: return "semicolon";
	case KEY_APOSTROPHE: return "apostrophe";
	case KEY_LBRACKET: return "lbracket";
	case KEY_RBRACKET: return "rbracket";

	case KEY_ENTER: return "enter";
	case KEY_BACKSPACE: return "backspace";
	case KEY_TILDE: return "tilde";
	default: return "Invalid Key";
	}
}

/// <summary>Sets a key's state and handles events.</summary>
/// <param name="key">The key id to set. ie: KEY_SPACE.</param>
/// <param name="state">The key state. True means pressed, false means released.</param>
void InputSystem::SetKey(int key, bool state) {

	bool isEvent = false;
	float time;
	bool keypressed = false;

	if (state && keyPressDuration[key] <= 0) {
		time = game.currentTime + keyPressDuration[key];
		keyPressDuration[key] = game.currentTime;
		isEvent = true;
		keypressed = true;
	}
	else if (!state && keyPressDuration[key] >= 0) {
		time = game.currentTime - keyPressDuration[key];
		keyPressDuration[key] = -game.currentTime;
		isEvent = true;
	}

	if (isEvent) {
		ControlContainer *container = keyPress[key];
		while (container != nullptr) {
			// This only works for actions. States and Axes are done in LoopControls
			if (container->control != nullptr && container->control->type == 3) {
				ActionControl *control = (ActionControl *)container->control;
				// Key Pressed
				if (keypressed && control->status == KEY_PRESSED)
					control->Invoke(time);
				// Key Released
				else if (!keypressed && control->status == KEY_RELEASED)
					control->Invoke(time);
			}

			container = container->next;
		}

		for (size_t i = 0; i < componentList.size(); i++) {
			componentList[i]->CallKeyEvent(key);
		}
	}
}

/// <summary><para>Checkes if a key is being pressed.</para>
///		<returns>Returns whether the button is being pressed.</returns>
/// </summary>
/// <param name="key">The key to be checked. ie: KEY_SPACE.</param>
bool InputSystem::CheckKey(int key) {
	if (key >= 0 && key < KEY_LAST)
		return keyPressDuration[key] > 0;
	return false;
}

bool InputSystem::CheckMouseButton(int button) {
	if (button >= 0 && button <= MOUSE_MOUSE5)
		return mousePressDuration[button] > 0;
	return false;
}

void InputSystem::SetMouseInWindow(bool resp) {
	mouseInWindow = resp;
}

bool InputSystem::IsMouseInWindow() {
	return mouseInWindow;
}

void InputSystem::SetFocused(bool resp) {
	focused = resp;
}

bool InputSystem::IsFocused() {
	return focused;
}

void InputSystem::SetMouseButton(int button, bool state) {
	bool isEvent = false;
	float time;
	KEY_STATUS keyStatus;

	if (state && mousePressDuration[button] <= 0) {
		time = game.currentTime + mousePressDuration[button];
		mousePressDuration[button] = game.currentTime;
		isEvent = true;
		keyStatus = KEY_PRESSED;
	}
	else if (!state && mousePressDuration[button] >= 0) {
		time = game.currentTime - mousePressDuration[button];
		mousePressDuration[button] = -game.currentTime;
		isEvent = true;
		keyStatus = KEY_RELEASED;
	}

	if (isEvent) {
		ControlContainer *container = mousePress[button];
		while (container != nullptr) {
			// This only works for actions. States and Axes are done in LoopControls
			if (container->control != nullptr && container->control->type == 3) {
				ActionControl *control = (ActionControl *)container->control;
				if (keyStatus == control->status)
					control->Invoke(time);
			}

			container = container->next;
		}

		for (size_t i = 0; i < componentList.size(); i++) {
			componentList[i]->CallMouseEvent(button);
		}
	}
}

void InputSystem::ResetCursor() {
	game.window->ResetCursor();
}

float InputSystem::GetKeyDuration(int key)
{
	return keyPressDuration[key];
}

float InputSystem::GetMouseDuration(int key)
{
	return mousePressDuration[key];
}

void InputSystem::LoopControls(double deltaTime) {
	if (!IsFocused())
		return;

	game.window->GetCursor(xpos, ypos);

	for (size_t i = 0; i < MOUSE_MOUSE5; i++) {
		if (mousePressDuration[i] > 0) { // Key is Pressed
			ControlContainer *container = mousePress[i];
			while (container != nullptr) {
				if (container->control != nullptr) {
					// Axis
					if (container->control->type == 1)
						container->control->Invoke(container->data);
					// State
					else if (container->control->type == 2)
						container->control->Invoke(mousePressDuration[i]);
				}

				container = container->next;
			}
		}
	}

	if (mousePress[MOUSE_XCOORD] != nullptr)
		if (mousePress[MOUSE_XCOORD]->control != nullptr)
			mousePress[MOUSE_XCOORD]->control->Invoke(float(deltaTime * (game.settings.resolution.x / 2 - xpos)));

	if (mousePress[MOUSE_YCOORD] != nullptr)
		if (mousePress[MOUSE_YCOORD]->control != nullptr)
			mousePress[MOUSE_YCOORD]->control->Invoke(float(deltaTime * (game.settings.resolution.y / 2 - ypos)));

	for (size_t i = 0; i < KEY_LAST; i++) {
		if (keyPressDuration[i] > 0) { // Key is Pressed
			ControlContainer *container = keyPress[i];
			while (container != nullptr) {
				if (container->control != nullptr) {
					// Axis
					if (container->control->type == 1) {
						container->control->Invoke(float(deltaTime*container->data));
					}
					// State
					else if (container->control->type == 2)
						container->control->Invoke(float(deltaTime));
				}

				container = container->next;
			}
		}
	}

	for (size_t i = 0; i < componentList.size(); i++) {
		componentList[i]->LoopControls(deltaTime);
	}

	game.window->ResetCursor();
}

InputComponent::InputComponent(std::string cfgFilePath, InputSystem *sys, EBaseEntity * e, System *s) :SuperComponent(e, s) {
	system = sys;
	componentType = COMPONENT_INPUT;

	for (size_t i = 0; i < MOUSE_LAST; i++)
		mousePress[i] = nullptr;
	for (size_t i = 0; i < KEY_LAST; i++)
		keyPress[i] = nullptr;

	ReadFile(cfgFilePath);
}

void InputComponent::CallKeyEvent(int key) {
	float time = system->GetKeyDuration(key);
	bool keypressed = time > 0;
	ControlContainer *container = keyPress[key];
	while (container != nullptr) {
		// This only works for actions. States and Axes are done in LoopControls
		if (container->control != nullptr && container->control->type == 3) {
			ActionControl *control = (ActionControl *)container->control;
			// Key Pressed
			if (keypressed && control->status == KEY_PRESSED)
				control->Invoke(time);
			// Key Released
			else if (!keypressed && control->status == KEY_RELEASED)
				control->Invoke(time);
		}

		container = container->next;
	}
}

void InputComponent::CallMouseEvent(int button) {
	if (button >= 0 && button <= MOUSE_MOUSE5) {
		float time = system->GetMouseDuration(button);
		bool buttonpressed = time > 0;

		ControlContainer *container = mousePress[button];
		while (container != nullptr) {
			// This only works for actions. States and Axes are done in LoopControls
			if (container->control != nullptr && container->control->type == 3) {
				ActionControl *control = (ActionControl *)container->control;
				// Key Pressed
				if (buttonpressed && control->status == KEY_PRESSED)
					control->Invoke(time);
				// Key Released
				else if (!buttonpressed && control->status == KEY_RELEASED)
					control->Invoke(time);
			}

			container = container->next;
		}
	}
}

void InputComponent::ReadFile(std::string path) {
	CFile input(path);
	cfgPath = path;

	if (input.fail()) {
		std::cout << "Failed to load " << path << "\n";
		return;
	}

	while (!input.null()) {
		std::string command;
		std::string control;
		int controlID;
		float data;

		command = strToLower(input.streamStr());
		control = input.streamStr();
		data = input.streamFloat();
		controlID = system->GetMouseIDByName(control);

		if (controlID != -1)
			AddCommand(command.c_str(), mousePress, controlID, data);
		else {
			controlID = system->GetKeyIDByName(control);
			if (controlID != -1)
				AddCommand(command.c_str(), keyPress, controlID, data);
		}
	}
	input.close();
}

void InputComponent::AddCommand(const char *str, ControlContainer **ctrl, int i, float data) {
	ControlContainer *newCtrl = ctrl[i];
	if (newCtrl == nullptr) {
		ctrl[i] = new ControlContainer(str, nullptr);
		ctrl[i]->data = data;
	}
	else {
		do {
			newCtrl = newCtrl->next;
		} while (newCtrl->next != nullptr);

		newCtrl = new ControlContainer(str, nullptr);
		newCtrl->data = data;
	}
}

void InputComponent::LinkCommand(BaseControl *ctrl) {
	std::string command = strToLower(ctrl->control);
	for (size_t i = 0; i < KEY_LAST; i++) {
		ControlContainer *container = keyPress[i];
		while (container != nullptr) {
			if (container->command == command)
				container->control = ctrl;

			container = container->next;
		}
	}

	for (size_t i = 0; i < MOUSE_LAST; i++) {
		ControlContainer *container = mousePress[i];
		while (container != nullptr) {
			if (container->command == command) {
				container->control = ctrl;
			}

			container = container->next;
		}
	}
}

void InputComponent::LoopControls(double deltaTime) {
	for (int i = 0; i < MOUSE_MOUSE5; i++) {
		if (system->GetMouseDuration(i) > 0) { // Key is Pressed
			ControlContainer *container = mousePress[i];
			while (container != nullptr) {
				if (container->control != nullptr) {
					// Axis
					if (container->control->type == 1)
						container->control->Invoke(float(deltaTime*container->data));
					// State
					else if (container->control->type == 2)
						container->control->Invoke(float(deltaTime));
				}

				container = container->next;
			}
		}
	}

	if (mousePress[MOUSE_XCOORD] != nullptr)
		if (mousePress[MOUSE_XCOORD]->control != nullptr)
			mousePress[MOUSE_XCOORD]->control->Invoke(float(deltaTime * (game.settings.resolution.x / 2 - system->xpos)));

	if (mousePress[MOUSE_YCOORD] != nullptr)
		if (mousePress[MOUSE_YCOORD]->control != nullptr)
			mousePress[MOUSE_YCOORD]->control->Invoke(float(deltaTime * (game.settings.resolution.y / 2 - system->ypos)));

	for (int i = 0; i < KEY_LAST; i++) {
		if (system->GetKeyDuration(i) > 0) { // Key is Pressed
			ControlContainer *container = keyPress[i];
			while (container != nullptr) {
				if (container->control != nullptr) {
					// Axis
					if (container->control->type == 1) {
						container->control->Invoke(float(deltaTime*container->data));
					}
					// State
					else if (container->control->type == 2)
						container->control->Invoke(float(deltaTime));
				}

				container = container->next;
			}
		}
	}
}
#endif
