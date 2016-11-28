#if 0
//#ifndef _INPUT_H
#define _INPUT_H

#include "InputInterface.h"
#include <vector>
#include <string>
#include <functional>

// Structs
class BaseControl {
public:
	const char *control;
	BaseClass *targetEntity;
	std::function<void(float)> function;
	int type;
	virtual void Invoke(float time);
	BaseControl(const char *Control, BaseClass *Target, std::function<void(float)> function);
};

class ActionControl : public BaseControl {
public:
	KEY_STATUS status;
	ActionControl(const char *Control, BaseClass *TargetEntity, std::function<void(float)> function, KEY_STATUS keyStatus);
};

class AxisControl : public BaseControl {
public:
	float minimum;
	float maximum;
	AxisControl(const char *Control, BaseClass *TargetEntity, std::function<void(float)> function, float Minimum, float Maximum);
};

class StateControl : public BaseControl {
public:
	StateControl(const char *Control, BaseClass *TargetEntity, std::function<void(float)> function);
};

struct ControlContainer {
	std::string command;
	BaseControl *control;
	ControlContainer *next;
	// Represents the amount of time a button has been pressed or the scale of the axis
	float data;

	ControlContainer() {
		command = "";
		control = nullptr;
		next = nullptr;
		data = 0;
	};

	ControlContainer(std::string cmd, BaseControl *ctrl) {
		command = cmd;
		control = ctrl;
		next = nullptr;
		data = 0;
	};
};

class InputComponent;

class InputSystem {
private:
	ControlContainer *keyPress[KEY_LAST];
	ControlContainer *mousePress[MOUSE_LAST];
	ControlContainer *windowFocus;

	// Represents the amount of time a button has been pressed. 
	//    +ve represents press, -ve represents release. 
	float keyPressDuration[KEY_LAST];
	float mousePressDuration[MOUSE_MOUSE5];

	bool focused;
	bool mouseInWindow;

	// Action Arrays
	std::vector<ActionControl *>	actionList;
	std::vector<AxisControl *>		axisList;
	std::vector<StateControl *>		stateList;

	std::vector<InputComponent *> componentList;

	void AddCommand(const char *, ControlContainer **, int, float);
	void LinkCommand(BaseControl *);
public:
	InputSystem();
	std::string getKeyString(int key);
	void SetMouseButton(int, bool);
	void SetKey(int, bool);
	void ResetCursor();

	float GetKeyDuration(int key);
	float GetMouseDuration(int key);
	bool CheckKey(int key);
	bool CheckMouseButton(int button);
	int GetKeyIDByName(std::string Control);
	int GetMouseIDByName(std::string Control);
	void CreateNewComponent(std::string, EBaseEntity *);

	void LoopControls(double);

	void SetMouseInWindow(bool);
	bool IsMouseInWindow();
	void SetFocused(bool);
	bool IsFocused();
	void ReadFile(std::string);

	int xpos, ypos;

	// Bind Functions
	template <typename T>
	void BindAction(const char *control, T *targetEntity, void (T::*methodPointer)(float), KEY_STATUS status = KEY_PRESSED) {
		std::function<void(float)> a = [=](float in) { (targetEntity->*methodPointer)(in); };
		actionList.push_back(new ActionControl(control, targetEntity, a, status));
		LinkCommand(actionList[actionList.size() - 1]);
	}

	template <typename T>
	void BindAxis(const char *control, T *targetEntity, void(T::*methodPointer)(float), float minimum = -1, float maximum = 1) {
		std::function<void(float)> a = [=](float in) { (targetEntity->*methodPointer)(in); };
		axisList.push_back(new AxisControl(control, targetEntity, a, minimum, maximum));
		LinkCommand(axisList[axisList.size() - 1]);
	}

	template <typename T>
	void BindState(const char *control, T *targetEntity, void (T::*methodPointer)(float)) {
		std::function<void(float)> a = [=](float in) { (targetEntity->*methodPointer)(in); };
		stateList.push_back(new StateControl(control, targetEntity, a));
		LinkCommand(stateList[stateList.size() - 1]);
	}
};

class InputComponent {
private:
	InputSystem *system;
	ControlContainer *keyPress[KEY_LAST];
	ControlContainer *mousePress[MOUSE_LAST];

	// Action Arrays
	std::vector<ActionControl *>	actionList;
	std::vector<AxisControl *>		axisList;
	std::vector<StateControl *>		stateList;


	void AddCommand(const char *, ControlContainer **, int, float);
	void LinkCommand(BaseControl *);
	void ReadFile(std::string);
	std::string cfgPath;
public:
	InputComponent(std::string, InputSystem *, EBaseEntity *, System *);
	void CallKeyEvent(int key);
	void CallMouseEvent(int key);
	void LoopControls(double);

	// Bind Functions
	template <typename T>
	void BindAction(const char *control, T *targetEntity, void (T::*methodPointer)(float), KEY_STATUS status = KEY_PRESSED) {
		std::function<void(float)> a = [=](float in) { (targetEntity->*methodPointer)(in); };
		actionList.push_back(new ActionControl(control, targetEntity, a, status));
		LinkCommand(actionList[actionList.size() - 1]);
	}

	template <typename T>
	void BindAxis(const char *control, T *targetEntity, void(T::*methodPointer)(float), float minimum = -1, float maximum = 1) {
		std::function<void(float)> a = [=](float in) { (targetEntity->*methodPointer)(in); };
		axisList.push_back(new AxisControl(control, targetEntity, a, minimum, maximum));
		LinkCommand(axisList[axisList.size() - 1]);
	}

	template <typename T>
	void BindState(const char *control, T *targetEntity, void (T::*methodPointer)(float)) {
		std::function<void(float)> a = [=](float in) { (targetEntity->*methodPointer)(in); };
		stateList.push_back(new StateControl(control, targetEntity, a));
		LinkCommand(stateList[stateList.size() - 1]);
	}
};

#endif