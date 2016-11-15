#include "Engine.h"
#include <stdio.h>
#include <chrono>
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
	if (!pfnCreateWindow) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}
#elif defined (_WIN32)
	HMODULE dllHandle = LoadLibrary("bin/window.dll");

	if (!dllHandle) {
		fprintf(stderr, "Failed to load window.dll!\n");
		return false;
	}

	GameWindow* (*pfnCreateWindow)();
	pfnCreateWindow = (GameWindow* (*)())GetProcAddress(dllHandle, "createWindow");

	if (!pfnCreateWindow) {
		fprintf(stderr, "Cannot get createWindow function!\n");
		return false;
	}
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

	GraphicsWrapper* (*pfnCreateGraphics)();
	pfnCreateGraphics = (GraphicsWrapper* (*)())dlsym(lib_handle, "createGraphics");

	if (!pfnCreateGraphics) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	Display* display;
	Window *win_handle;
	Screen *screen;
	int screenID;
	std::cout << "Getting Handles\n";
	window->GetHandles(display, win_handle, screen, screenID);
	std::cout << "Handles gotten\n";
#elif defined (_WIN32)
	HMODULE dllHandle = LoadLibrary("bin/opengl.dll");

	if (!dllHandle) {
		fprintf(stderr, "Failed to load window.dll!\n");
		return false;
	}

	GraphicsWrapper* (*pfnCreateGraphics)();
	pfnCreateGraphics = (GraphicsWrapper* (*)())GetProcAddress(dllHandle, "createGraphics");

	if (!pfnCreateGraphics) {
		fprintf(stderr, "Cannot get createGraphics function!\n");
		return false;
	}

	HWND win_handle = window->GetHandle();
#endif
	
	std::cout << "Creating GraphicsWrapper\n";
	std::cout << pfnCreateGraphics << "\n";
	graphicsWrapper = (GraphicsWrapper*)pfnCreateGraphics();
	std::cout << "Passing Context\n";
#if defined (__linux__)
	graphicsWrapper->SetWindowContext(display, win_handle, screen, screenID);
#elif defined (_WIN32)
	graphicsWrapper->SetWindowContext(win_handle);
#endif
	std::cout << "Context Passed\n";

	if (!graphicsWrapper->InitializeWindowContext())
		return false;

	std::cout << "WindowContext Initialized\n";

	if (!graphicsWrapper->InitializeGraphics())
		return false;
	std::cout << "Graphics Initialized\n";
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
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime, prevTime;
	prevTime = std::chrono::high_resolution_clock::now();

	std::chrono::microseconds delta;
	while (isRunning) {
		currentTime = std::chrono::high_resolution_clock::now();
		delta = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime);
		//std::cout << 1000000.0/delta.count() << "\n";
		prevTime = currentTime;

		window->HandleEvents();
	}
}

void Engine::Shutdown() {
	isRunning = false;
}

Engine::~Engine() {
	std::cout << "Shutting Down\n";
	window->Shutdown();
	std::cout << "Window Down Down\n";
}
