#ifndef _INPUT_H
#define _INPUT_H

#include "Systems/BaseSystem.hpp"
#include "InputInterface.hpp"
#include <functional>
#include <vector>

class InputComponent;

enum COMMAND_TYPE {
	COMMAND_BASE = 0,
	COMMAND_ACTION,
	COMMAND_AXIS
};

// Structs
class BaseCommand {
public:
	void *targetEntity;
	std::function<void(double)> function;
	COMMAND_TYPE type;
	virtual void Invoke(double value);
	BaseCommand(void *Target, std::function<void(double)> function);
};

class ActionCommand : public BaseCommand {
public:
	KEY_STATUS status;
	ActionCommand(void *TargetEntity, std::function<void(double)> function, KEY_STATUS keyStatus);
};

class AxisCommand : public BaseCommand {
public:
	AxisCommand(void *TargetEntity, std::function<void(double)> function);
};

struct ControlHandler {
	BaseCommand *command;

	std::string control;
	InputComponent *component;

	double value;

	ControlHandler *nextControl;
	ControlHandler *prevControl;
	ControlHandler(std::string controlCode, InputComponent *componentPtr, ControlHandler *prev, double val);
};

class InputManager : public InputInterface {
protected:
	std::vector<ControlHandler *> keyboardControls;
	std::vector<ControlHandler *> mouseControls;
	std::vector<ControlHandler *> windowControls;
	std::vector<ControlHandler *> allControls;
	std::vector<double> keyboardData;
	std::vector<double> mouseData;
	std::vector<double> windowData;
	bool useKeyboard;
	bool useMouse;
	bool useWindow;
public:
	void ResizeEvent(int, int);
	void SetMouseButton(int, bool);
	void SetMousePosition(int, int);
	void GetMousePosition(int &, int &);
	void SetFocused(bool);
	bool IsFocused();
	void SetKey(int, bool);
	void Quit();
	void ForceQuit();
	int GetKey(int key);
	int GetMouseButton(int mb);

	InputManager();
	void LoopControls(double deltaTime);

	int GetKeyboardKeyByName(std::string key);
	int GetMouseKeyByName(std::string key);
	int GetWindowKeyByName(std::string key);
	int TranslateKey(std::string key, int &device, ControlHandler *&handler);
	bool AddControl(std::string key, std::string control, InputComponent *component, double val);
	ControlHandler *GetControl(std::string control, InputComponent *component);

	void SetInputControlFile(std::string path);
	void SetInputControlFile(std::string path, InputComponent *component);

	void Cleanup();
	void Shutdown();
	void Cleanup(InputComponent *component);
	void CleanupSystem(InputComponent * component, bool use, std::vector<ControlHandler *> &list);

	template <typename T>
	void BindAction(std::string control, InputComponent *component, T *targetEntity, void (T::*methodPointer)(double), KEY_STATUS status = KEY_PRESSED) {
		std::function<void(double)> a = [=](double in) { (targetEntity->*methodPointer)(in); };
		for (size_t i = 0; i < allControls.size(); i++) {
			if (allControls[i]->component == component && allControls[i]->control == control)
				allControls[i]->command = new ActionCommand(targetEntity, a, status);
		}
	}

	template <typename T>
	void BindAxis(std::string control, InputComponent *component, T *targetEntity, void (T::*methodPointer)(double)) {
		std::function<void(double)> a = [=](double in) { (targetEntity->*methodPointer)(in); };
		for (size_t i = 0; i < allControls.size(); i++) {
			if (allControls[i]->component == component && allControls[i]->control == control)
				allControls[i]->command = new AxisCommand(targetEntity, a);
		}
	}
};

class InputComponent {
public:
	~InputComponent();
	void SetInputControlFile(std::string path);
	template <typename T>
	void BindAction(std::string control, T *targetEntity, void (T::*methodPointer)(double), KEY_STATUS status = KEY_PRESSED) {
		if (manager_ != NULL)
			manager_->BindAction(control, this, targetEntity, methodPointer, status);
	}

	template <typename T>
	void BindAxis(std::string control, T *targetEntity, void (T::*methodPointer)(double)) {
		if (manager_ != NULL)
			manager_->BindAxis(control, this, targetEntity, methodPointer);
	}
private:
	InputManager *manager_;
};

#endif
