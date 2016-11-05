#include "Engine.h"
#include <stdio.h>
#ifndef _MSC_VER
#include <dlfcn.h>
#endif

#ifdef UseClassInstance
	Engine *Engine::_instance=0;
#endif

bool Engine::Initialize() {
	if (!InitializeWindow())
		return false;


	isRunning = true;
	return true;
}

bool Engine::InitializeWindow() {
#ifdef __linux__
	void *lib_handle = dlopen("./bin/window.so", RTLD_LAZY);

	if (!lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}
	GameWindow* (*createWindow)();

	createWindow = (GameWindow* (*)())dlsym(lib_handle, "createWindow");
#endif

	window = (GameWindow*)createWindow();
	if (!window->Initialize("The Grind Engine", 1024, 768))
		return false;
	
	inputInterface = new InputInterface();
	window->SetInputPointer(inputInterface);
	
	return true;
}

#ifdef UseClassInstance
Engine *Engine::GetInstance() {
	if (!_instance)
		_instance = new Engine();
	return _instance;
}
#else
Engine &Engine::GetInstance() {
	static Engine newEngine;
	return newEngine;
}
#endif

void Engine::Run() {
	while (isRunning) {
		window->HandleEvents();
	}
}

void Engine::Shutdown() {
	isRunning = false;
}

Engine::~Engine() {
	std::cout << "Shutting Down" << std::endl;
	window->Shutdown();
	std::cout << "Window Down Down" << std::endl;
	delete inputInterface;
	std::cout << "Interface Down" << std::endl;
}
