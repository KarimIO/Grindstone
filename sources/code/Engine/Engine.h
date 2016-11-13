#ifndef _ENGINE_H
#define _ENGINE_H

#include "BaseClass.h"
#include "InputInterface.h"
#include <Window.h>
#include <Graphics.h>

class Engine : public BaseClass {
private:
	GameWindow *window;
	bool InitializeWindow();
public:
	InputInterface inputInterface;
	//InputSystem *inputSystem;
#ifdef UseClassInstance
	static Engine *GetInstance();
#else
	static Engine &GetInstance();
#endif
	bool Initialize();
	void Run();
	void Shutdown();

	bool isRunning;

	~Engine();
};

#ifdef UseClassInstance
	#define engine (*Engine::GetInstance())	// Needs work
#else
	#define engine Engine::GetInstance()	// Only works with C++11+
#endif

#endif