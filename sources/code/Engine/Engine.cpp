#include "Engine.h"
#include <stdio.h>
#ifndef _WIN32
#include <dlfcn.h>
#endif

#ifdef UseClassInstance
	Engine *Engine::_instance=0;
#endif

bool Engine::Initialize() {
	if (!InitializeWindow())	return false;
	if (!InitializeGraphics())	return false;

	graphicsWrapper->SwapBuffer();

	isRunning = true;
	return true;
}

bool Engine::InitializeWindow() {
#if defined (__linux__)
	void *lib_handle = dlopen("./bin/window.so", RTLD_LAZY);

	if (!lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	GameWindow* (*pfnCreateWindow)();
	pfnCreateWindow = (GameWindow* (*)())dlsym(lib_handle, "createWindow");
#elif defined (_WIN32)
	HMODULE dllHandle = LoadLibrary("bin/window.dll");

	if (!dllHandle) {
		fprintf(stderr, "Failed to load window.dll!\n");
		return false;
	}

	GameWindow* (*pfnCreateWindow)();
	pfnCreateWindow = (GameWindow* (*)())GetProcAddress(dllHandle, "createWindow");
#endif

	window = (GameWindow*)pfnCreateWindow();
	if (!window->Initialize("The Grind Engine", 1024, 768))
		return false;

	window->SetInputPointer(&inputInterface);

	return true;
}

bool Engine::InitializeGraphics() {
#if defined (__linux__)
	void *lib_handle = dlopen("./bin/opengl.so", RTLD_LAZY);

	if (!lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	GameWindow* (*pfnCreateWindow)();
	pfnCreateWindow = (GameWindow* (*)())dlsym(lib_handle, "createWindow");
#elif defined (_WIN32)
	HMODULE dllHandle = LoadLibrary("bin/opengl.dll");

	if (!dllHandle) {
		fprintf(stderr, "Failed to load window.dll!\n");
		return false;
	}

	GraphicsWrapper* (*pfnCreateWindow)();
	pfnCreateWindow = (GraphicsWrapper* (*)())GetProcAddress(dllHandle, "createGraphics");

	HWND handle = window->GetHandle();
#endif
	
	graphicsWrapper = (GraphicsWrapper*)pfnCreateWindow();
	graphicsWrapper->SetWindowContext(handle);

	if (!graphicsWrapper->InitializeWindowContext())
		return false;

	if (!graphicsWrapper->InitializeGraphics())
		return false;
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
}
