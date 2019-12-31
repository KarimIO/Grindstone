#include "Input.hpp"
#include "Engine.hpp"
#include "Utilities.hpp"

#include "../GraphicsCommon/GraphicsWrapper.hpp"
#include "AssetManagers/ImguiManager.hpp"

InputManager::InputManager() {
	GRIND_PROFILE_FUNC();
	// These should be set elsewhere
	useWindow = true;
	useKeyboard = true;
	useMouse = true;

	if (useKeyboard) {
		keyboardControls.resize(KEY_LAST);
		keyboardData.resize(KEY_LAST);
	}

	if (useMouse) {
		mouseControls.resize(MOUSE_LAST);
		mouseData.resize(MOUSE_LAST);
	}

	if (useWindow) {
		windowControls.resize(WINDOW_LAST);
		windowData.resize(WINDOW_LAST);
	}

	AddControl("escape", "Shutdown", NULL, 1);
	BindAction("Shutdown", NULL, &engine, &Engine::shutdownControl, KEY_RELEASED);

	AddControl("f7", "RefreshAll", NULL, 1);
	BindAction("RefreshAll", NULL, &engine, &Engine::refreshAll, KEY_RELEASED);

	AddControl("f8", "Editor", NULL, 1);
	BindAction("Editor", NULL, &engine, &Engine::editorControl, KEY_RELEASED);

	AddControl("f9", "ProfileFrame", NULL, 1);
	BindAction("ProfileFrame", NULL, &engine, &Engine::profileFrame, KEY_RELEASED);

}

int InputManager::GetKeyboardKeyByName(std::string control) {
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

int InputManager::GetMouseKeyByName(std::string control) {
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

int InputManager::GetWindowKeyByName(std::string control) {
	if (control == "winclose")		return WINDOW_CLOSE;
	if (control == "winforceclose")	return WINDOW_FORCECLOSE;
	if (control == "winresizex")	return WINDOW_RESIZEX;
	if (control == "winresizey")	return WINDOW_RESIZEY;

	if (control == "winmovex")		return WINDOW_MOVEX;
	if (control == "winmovey")		return WINDOW_MOVEY;
	if (control == "winfocus")		return WINDOW_FOCUS;
	if (control == "winminimize")	return WINDOW_MINIMIZE;
	return -1;
}

int InputManager::TranslateKey(std::string key, int &device, ControlHandler *&handler) {
	std::string control = strToLower(key);
	int k;

	if (useWindow) {
		k = GetWindowKeyByName(control);
		if (k != -1) {
			device = DEVICE_WINDOW;
			handler = windowControls[k];
			return k;
		}
	}

	if (useMouse) {
		k = GetMouseKeyByName(control);
		if (k != -1) {
			device = DEVICE_MOUSE;
			handler = mouseControls[k];
			return k;
		}
	}

	if (useKeyboard) {
		k = GetKeyboardKeyByName(control);
		if (k != -1) {
			device = DEVICE_KEYBOARD;
			handler = keyboardControls[k];
			return k;
		}
	}

	handler = NULL;
	return -1;
}

bool InputManager::AddControl(std::string keyCode, std::string controlCode, InputComponent *component, double val) {
	ControlHandler *handler;
	int device, key;
	// Get Key and device from string
	key = TranslateKey(keyCode, device, handler);
	if (key == -1) {
		GRIND_ERROR("INPUT: {0} is not a supported keycode!", keyCode.c_str());
		return false;
	}

	// If we have no controls, add one.
	if (handler == NULL) {
		if (device == DEVICE_WINDOW) {
			windowControls[key] = new ControlHandler(controlCode, component, NULL, val);
			allControls.push_back(windowControls[key]);
			return true;
		}
		else if (device == DEVICE_MOUSE) {
			mouseControls[key] = new ControlHandler(controlCode, component, NULL, val);
			allControls.push_back(mouseControls[key]);
			return true;
		}
		else if (device == DEVICE_KEYBOARD) {
			keyboardControls[key] = new ControlHandler(controlCode, component, NULL, val);
			allControls.push_back(keyboardControls[key]);
			return true;
		}
		else
			return false;
	}

	// Make sure we have no instance of the control-component combination
	if (handler->component == component && handler->control == controlCode)
		return false;

	while (handler->nextControl != NULL) {
		if (handler->component == component && handler->control == controlCode)
			return false;
	}

	// If we have no matches, add one.
	handler->nextControl = new ControlHandler(controlCode, component, handler, val);
	allControls.push_back(handler->nextControl);
	return true;
}

ControlHandler * InputManager::GetControl(std::string control, InputComponent *component) {
	for (size_t i = 0; i < allControls.size(); i++) {
		if (allControls[i]->component == component && allControls[i]->control == control)
			return allControls[i];
	}

	return NULL;
}

void InputManager::SetInputControlFile(std::string path) {
	SetInputControlFile(path, NULL);
}

void InputManager::SetInputControlFile(std::string path, InputComponent * component) {
	std::ifstream file;

	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		file.open("../" + path);
	}
	catch (std::system_error& e) {
		std::cerr << e.code().message() << std::endl;
	}

	if (file.fail()) {
		GRIND_ERROR("INPUT: File load failed: {0}\n", path.c_str());
		return;
	}

	std::string command, key;
	double num;
	while (!file.eof()) {
		file >> key;
		file >> command;
		file >> num;
		AddControl(key, command, component, num);
	}

	file.close();
}

void InputManager::CleanupSystem(InputComponent * component, bool use, std::vector<ControlHandler *> &list) {
	if (use) {
		for (size_t i = 0; i < list.size(); i++) {
			ControlHandler *handler = list[i];
			if (handler != nullptr) {
				ControlHandler *first = nullptr;
				while (handler != nullptr) {
					if (handler->component == component) {
						ControlHandler *nextTemp = handler->nextControl;
						ControlHandler *prevTemp = handler->prevControl;
						if (nextTemp != NULL && prevTemp != NULL) {
							nextTemp->prevControl = prevTemp;
							prevTemp->nextControl = nextTemp;
						}
						delete handler;
						handler = nextTemp;
					}
					else {
						if (first == nullptr)
							first = handler;
						handler = handler->nextControl;
					}
				}
				list[i] = first;
			}
		}
	}
}

void InputManager::Cleanup() {
	for (size_t i = 0; i < allControls.size(); i++) {
		delete allControls[i];
	}

	// Only do if we're going to rebind the keys.
	if (useKeyboard) {
		for (size_t i = 0; i < keyboardControls.size(); i++) {
			keyboardControls[i] = NULL;
		}
	}

	if (useMouse) {
		for (size_t i = 0; i < mouseControls.size(); i++) {
			mouseControls[i] = NULL;
		}
	}

	if (useWindow) {
		for (size_t i = 0; i < windowControls.size(); i++) {
			windowControls[i] = NULL;
		}
	}
}

void InputManager::Shutdown() {
	for (size_t i = 0; i < allControls.size(); i++) {
		delete allControls[i];
	}
}

void InputManager::Cleanup(InputComponent * component) {
	// Dereference from allcontrols first
	for (size_t i = 0; i < allControls.size(); i++) {
		if (allControls[i]->component == component)
			allControls.erase(allControls.begin() + i);
	}

	// Sort out linked lists
	CleanupSystem(component, useKeyboard,	keyboardControls);
	CleanupSystem(component, useMouse,		mouseControls);
	CleanupSystem(component, useWindow,		windowControls);
}

void InputManager::ResizeEvent(int x, int y) {
	GRIND_LOG("Resized to: {0}, {1}.", x, y);
	engine.getSettings()->resolution_x_ = x;
	engine.getSettings()->resolution_y_ = y;
}

void InputManager::SetMousePosition(int x, int y) {
	if (!IsFocused())
		return;

	double delta_time = engine.getUpdateTimeDelta();

	double xOrg, yOrg;
	xOrg = mouseData[MOUSE_XCOORD] * delta_time;
	yOrg = mouseData[MOUSE_YCOORD] * delta_time;

	if ((xOrg - x) != 0) {
		mouseData[MOUSE_XCOORD] = x;
		ControlHandler *container = mouseControls[MOUSE_XCOORD];
		while (container != NULL) {
			if (container->command != NULL && container->command->type == COMMAND_AXIS)
				container->command->Invoke(engine.getSettings()->resolution_x_ / 2 - x);
			container = container->nextControl;
		}
	}

	if ((yOrg - y) != 0) {
		mouseData[MOUSE_YCOORD] = y;
		ControlHandler *container = mouseControls[MOUSE_YCOORD];
		while (container != NULL) {
			if (container->command != NULL && container->command->type == COMMAND_AXIS)
				container->command->Invoke(engine.getSettings()->resolution_y_ / 2 - y);
			container = container->nextControl;
		}
	}
}

void InputManager::GetMousePosition(int &x, int &y) {
	engine.getGraphicsWrapper()->GetCursor(x, y);
}

int InputManager::GetKey(int k) {
	return keyboardData[k] > 0.0;
}

int InputManager::GetMouseButton(int mb) {
	return mouseData[mb] > 0.0;
}

void InputManager::SetMouseButton(int mb, bool state) {
	if (!IsFocused() && state) // Only set buttons when unfocused if it's a release
		return;

	bool isEvent = false;
	double time;
	bool buttonpressed = false;

	if (mb >= MOUSE_LEFT && mb <= MOUSE_MOUSE5) {
		if (state && mouseData[mb] <= 0) {
			time = engine.getTimeCurrent() + mouseData[mb];
			mouseData[mb] = engine.getTimeCurrent();
			isEvent = true;
			buttonpressed = true;
		}
		else if (!state && mouseData[mb] >= 0) {
			time = engine.getTimeCurrent() - mouseData[mb];
			mouseData[mb] = -engine.getTimeCurrent();
			isEvent = true;
		}

		if (isEvent) {
			ControlHandler *container = mouseControls[mb];
			while (container != NULL) {
				// This only works for actions. States and Axes are done in LoopControls
				if (container->command != NULL && container->command->type == COMMAND_ACTION) {
					ActionCommand *control = (ActionCommand *)container->command;
					// Key Pressed
					if (buttonpressed && control->status == KEY_PRESSED)
						control->Invoke(time);
					// Key Released
					else if (!buttonpressed && control->status == KEY_RELEASED)
						control->Invoke(time);
				}

				container = container->nextControl;
			}
		}

		engine.getImguiManager()->MouseButtonCallback(mb, state, 0);
	}

	if (mb >= MOUSE_WHEEL_UP && mb <= MOUSE_WHEEL_RIGHT) {
		ControlHandler *container = mouseControls[mb];
		while (container != NULL) {
			if (container->command != NULL) {
				BaseCommand *control = (BaseCommand *)container->command;
				control->Invoke(engine.getTimeCurrent());
				container = container->nextControl;
			}
		}

		double y = (mb == MOUSE_WHEEL_UP) - (mb == MOUSE_WHEEL_DOWN);
		double x = (mb == MOUSE_WHEEL_RIGHT) - (mb == MOUSE_WHEEL_LEFT);
		
		engine.getImguiManager()->ScrollCallback(x, y);
	}
}

void InputManager::SetFocused(bool state) {
	bool isEvent = false;
	double time;
	bool buttonpressed = false;

	if (state) {
		time = engine.getTimeCurrent() + mouseData[WINDOW_FOCUS];
		windowData[WINDOW_FOCUS] = 1;
		isEvent = true;
		buttonpressed = true;
	}
	else if (!state) {
		time = engine.getTimeCurrent() - windowData[WINDOW_FOCUS];
		windowData[WINDOW_FOCUS] = 0;
		isEvent = true;
	}

	if (isEvent) {
		ControlHandler *container = windowControls[WINDOW_FOCUS];
		while (container != NULL) {
			// This only works for actions. States and Axes are done in LoopControls
			if (container->command != NULL && container->command->type == COMMAND_ACTION) {
				ActionCommand *control = (ActionCommand *)container->command;
				// Key Pressed
				if (buttonpressed && control->status == KEY_PRESSED)
					control->Invoke(time);
				// Key Released
				else if (!buttonpressed && control->status == KEY_RELEASED)
					control->Invoke(time);
			}

			container = container->nextControl;
		}
	}
}

bool InputManager::IsFocused() {
	return windowData[WINDOW_FOCUS] > 0;
}

void InputManager::SetKey(int key, bool state) {
	bool isEvent = false;
	double time;
	bool keypressed = false;

	if (key < 0 || key >= KEY_LAST)
		return;

	if (state && keyboardData[key] <= 0) {
		time = engine.getTimeCurrent() + keyboardData[key];
		keyboardData[key] = engine.getTimeCurrent();
		isEvent = true;
		keypressed = true;
	}
	else if (!state && keyboardData[key] >= 0) {
		time = engine.getTimeCurrent() - keyboardData[key];
		keyboardData[key] = -engine.getTimeCurrent();
		isEvent = true;
	}

	if (isEvent) {
		ControlHandler *container = keyboardControls[key];
		while (container != NULL) {
			// This only works for actions. States and Axes are done in LoopControls
			if (container->command != NULL && container->command->type == COMMAND_ACTION) {
				ActionCommand *control = (ActionCommand *)container->command;
				// Key Pressed
				if (keypressed && control->status == KEY_PRESSED)
					control->Invoke(time);
				// Key Released
				else if (!keypressed && control->status == KEY_RELEASED)
					control->Invoke(time);
			}

			container = container->nextControl;
		}

		engine.getImguiManager()->KeyCallback(key, state);
	}
}

void InputManager::Quit() {
	engine.getGraphicsWrapper()->Close();
}

void InputManager::ForceQuit() {
	engine.shutdown();
}

void InputManager::LoopControls(double deltaTime) {
	GRIND_PROFILE_FUNC();
	if (!IsFocused() || (engine.edit_mode_ && !engine.edit_is_simulating_))
		return;

	for (size_t i = 0; i <= MOUSE_MOUSE5; i++) {
		if (mouseData[i] > 0) { // Key is Pressed
			ControlHandler *container = mouseControls[i];
			while (container != NULL) {
				if (container->command != NULL && container->command->type == COMMAND_AXIS)
					container->command->Invoke(deltaTime * container->value);

				container = container->nextControl;
			}
		}
	}

	for (size_t i = 0; i < KEY_LAST; i++) {
		if (keyboardData[i] > 0) { // Key is Pressed
			ControlHandler *container = keyboardControls[i];
			while (container != NULL) {
				if (container->command != NULL && container->command->type == COMMAND_AXIS)
					container->command->Invoke(deltaTime * container->value);

				container = container->nextControl;
			}
		}
	}

	auto sett = engine.getSettings();
	if (!engine.edit_mode_)
		engine.getGraphicsWrapper()->SetCursor(sett->resolution_x_ / 2, sett->resolution_y_ / 2);
}

ControlHandler::ControlHandler(std::string controlCode, InputComponent * componentPtr, ControlHandler * prev, double val) {
	control = controlCode;
	component = componentPtr;
	prevControl = prev;
	nextControl = NULL;
	value = val;
	command = NULL;
}

BaseCommand::BaseCommand(void * Target, std::function<void(double)> callback) {
	type = COMMAND_BASE;
	targetEntity = Target;
	function = callback;
}


AxisCommand::AxisCommand(void * TargetEntity, std::function<void(double)> function) : BaseCommand(TargetEntity, function) {
	type = COMMAND_AXIS;
}

ActionCommand::ActionCommand(void * TargetEntity, std::function<void(double)> function, KEY_STATUS keyStatus) : BaseCommand(TargetEntity, function) {
	type = COMMAND_ACTION;
	this->status = keyStatus;
}

void BaseCommand::Invoke(double value) {
	if (function != NULL)
		function(value);
}

InputComponent::~InputComponent() {
	manager_->Cleanup(this);
}

void InputComponent::SetInputControlFile(std::string path) {
	manager_ = engine.getInputManager();
	manager_->SetInputControlFile(path, this);
}