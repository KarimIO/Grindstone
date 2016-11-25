#ifndef _ENGINE_H
#define _ENGINE_H

#include "BaseClass.h"
#include "InputInterface.h"
#include <Window.h>
#include <OGLGraphicsWrapper.h>
#include <GLShader.h>
#include "GeometryCache.h"

#include <chrono>
#include <string>

class Engine : public BaseClass {
private:
	GameWindow *window;
	bool InitializeWindow();
	bool InitializeGraphics();

	// Time Data
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime;

	std::chrono::microseconds deltaTime;
public:
	GeometryCache geometryCache;

	InputInterface inputInterface;
	GraphicsWrapper *graphicsWrapper;
	//InputSystem *inputSystem;
#ifdef UseClassInstance
	static Engine *GetInstance();
#else
	static Engine &GetInstance();
#endif
	bool Initialize();
	void Run();

	std::string GetAvailablePath(std::string);

	bool InitializeScene(std::string);
	void CalculateTime();

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