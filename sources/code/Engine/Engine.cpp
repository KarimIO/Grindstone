#include "Engine.h"
#include "Utilities.h"
#include "GraphicsDLLPointer.h"
#include "iniHandler.h"
#include <stdio.h>
#ifndef _WIN32
#include <dlfcn.h>
#endif

#ifdef UseClassInstance
	Engine *Engine::_instance=0;
#endif

VertexArrayObject*	(*pfnCreateVAO)();
VertexBufferObject*	(*pfnCreateVBO)();
ShaderProgram*		(*pfnCreateShader)();
Texture*			(*pfnCreateTexture)();
Framebuffer*		(*pfnCreateFramebuffer)();

bool Engine::Initialize() {
	srand(time(NULL));

	InitializeAudio();
	// Get Settings here:
	InitializeSettings();
	if (!InitializeWindow())						return false;
	if (!InitializeGraphics(GRAPHICS_OPENGL))		return false;

	// An array of 3 vectors which represents 3 vertices
	float g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};

	std::string vsPath = "../shaders/objects/main.glvs"; // GetShaderExt()
	std::string fsPath = "../shaders/objects/mainMetalness.glfs";
		
	std::string vsContent;
	if (!ReadFile(vsPath, vsContent))
		fprintf(stderr, "Failed to read vertex shader: %s.\n", vsPath.c_str());

	std::string fsContent;
	if (!ReadFile(fsPath, fsContent))
		fprintf(stderr, "Failed to read fragment shader: %s.\n", fsPath.c_str());

	shader = pfnCreateShader();
	shader->Initialize(2);
	if (!shader->AddShader(&vsPath, &vsContent, SHADER_VERTEX))
		fprintf(stderr, "Failed to add vertex shader %s.\n", vsPath.c_str());
	if (!shader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT))
		fprintf(stderr, "Failed to add fragment shader %s.\n", vsPath.c_str());
	shader->Compile();

	shader->SetNumUniforms(7);
	shader->CreateUniform("pvmMatrix");
	shader->CreateUniform("modelMatrix");
	shader->CreateUniform("viewMatrix");
	shader->CreateUniform("tex0");
	shader->CreateUniform("tex1");
	shader->CreateUniform("tex2");
	shader->CreateUniform("tex3");

	vsContent.clear();
	fsContent.clear();


	if (!InitializeScene("../scenes/startup.gmf"))	return false;
	cubemapSystem.LoadCubemaps();

	renderPathType = RENDERPATH_DEFERRED;
	switch (renderPathType) {
	default:
		renderPath = (RenderPath *)new RenderPathForward(graphicsWrapper, &geometryCache);
		break;
	case RENDERPATH_DEFERRED:
		renderPath = (RenderPath *)new RenderPathDeferred(graphicsWrapper, &geometryCache);
		break;
	};

	lightSystem.SetPointers(graphicsWrapper, &geometryCache);

	engine.inputSystem.AddControl("escape", "Shutdown", NULL, 1);
	engine.inputSystem.BindAction("Shutdown", NULL, &engine, &Engine::ShutdownControl);

	engine.inputSystem.AddControl("e", "PlaySound", NULL, 1);
	engine.inputSystem.BindAction("PlaySound", NULL, &engine, &Engine::PlayEngineSound);

	engine.inputSystem.AddControl("r", "PlaySound2", NULL, 1);
	engine.inputSystem.BindAction("PlaySound2", NULL, &engine, &Engine::PlayEngineSound2);

	engine.inputSystem.AddControl("q", "CaptureCubemaps", NULL, 1);
	engine.inputSystem.BindAction("CaptureCubemaps", NULL, &(engine.cubemapSystem), &CubemapSystem::CaptureCubemaps);

	isRunning = true;
	prevTime = std::chrono::high_resolution_clock::now();
	startTime = std::chrono::high_resolution_clock::now();
	return true;
}

void Engine::InitializeSettings() {
	INIConfigFile cfile;
	
	if (cfile.Initialize("../settings.ini")) {
		cfile.GetInteger("Window", "resx",	1024,	settings.resolutionX);
		cfile.GetInteger("Window", "resy",	768,	settings.resolutionY);
		cfile.GetFloat(  "Window", "fov",	90,		settings.fov);
		settings.fov *= 3.14159f / 360.0f; // Convert to rad, /2 for full fovY.
		std::string graphics;
		cfile.GetString("Renderer", "graphics", "OpenGL", graphics);

		graphics = strToLower(graphics);
		if (graphics == "directx")
			settings.graphicsLanguage = GRAPHICS_DIRECTX;
		else if (graphics == "vulkan")
			settings.graphicsLanguage = GRAPHICS_VULKAN;
		else if (graphics == "metal")
			settings.graphicsLanguage = GRAPHICS_METAL;
		else if (graphics == "opengl")
			settings.graphicsLanguage = GRAPHICS_OPENGL;
		else {
			fprintf(stderr, "SETTINGS.INI: Invalid value for graphics language (%s), using Opengl instead.\n", graphics.c_str());
			settings.graphicsLanguage = GRAPHICS_OPENGL;
			cfile.SetString("Renderer", "graphics", "OpenGL");
		}
	}
	else {
		fprintf(stderr, "SETTINGS.INI: File not found.\n");
		settings.resolutionX = 1024;
		settings.resolutionY = 768;
		settings.graphicsLanguage = GRAPHICS_OPENGL;
	}

}

bool Engine::InitializeWindow() {
#if defined (__linux__)
	void *lib_handle = dlopen("./window.so", RTLD_LAZY);

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
	HMODULE dllHandle = LoadLibrary("window.dll");

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
	if (!window->Initialize("The Grindstone Engine", settings.resolutionX, settings.resolutionY))
		return false;

	window->SetInputPointer(&inputSystem);
	window->SetCursorShown(false);

	return true;
}

bool Engine::InitializeAudio() {
#if defined (__linux__)
	void *lib_handle = dlopen("./audio.so", RTLD_LAZY);

	if (!lib_handle) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	AudioSystem* (*pfnCreateAudio)();
	pfnCreateAudio = (AudioSystem* (*)())dlsym(lib_handle, "createAudio");
	if (!pfnCreateAudio) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}
#elif defined (_WIN32)
	HMODULE dllHandle = LoadLibrary("audio.dll");

	if (!dllHandle) {
		fprintf(stderr, "Failed to load audio.dll!\n");
		return false;
	}

	AudioSystem *(*pfnCreateAudio)();
	pfnCreateAudio = (AudioSystem *(*)())GetProcAddress(dllHandle, "createAudio");

	if (!pfnCreateAudio) {
		fprintf(stderr, "Cannot get createAudio function!\n");
		return false;
	}
#endif

	audioSystem = pfnCreateAudio();
	audioSystem->Initialize();

	return true;
}

bool Engine::InitializeGraphics(GraphicsLanguage gl) {
	std::string library;
	switch (gl) {
	default:
		library = "opengl";
		break;
#ifndef __APPLE__
	case GRAPHICS_VULKAN:
		library = "vulkan";
		break;
#endif
#ifdef _WIN32
	case GRAPHICS_DIRECTX:
		library = "directx";
		break;
#endif
#ifdef __APPLE__
	case GRAPHICS_METAL:
		library = "metal";
		break;
#endif
	};
	
#if defined (__linux__)
	void *lib_handle = dlopen(("./"+ library +".so").c_str(), RTLD_LAZY);

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

	pfnCreateVAO = (VertexArrayObject* (*)())dlsym(lib_handle, "createVAO");
	if (!pfnCreateVAO) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	pfnCreateVBO = (VertexBufferObject* (*)())dlsym(lib_handle, "createVBO");
	if (!pfnCreateVBO) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	pfnCreateShader = (ShaderProgram* (*)())dlsym(lib_handle, "createShader");
	if (!pfnCreateGraphics) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	pfnCreateTexture = (Texture* (*)())dlsym(lib_handle, "createTexture");
	if (!pfnCreateTexture) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	pfnCreateFramebuffer = (Framebuffer* (*)())dlsym(lib_handle, "createFramebuffer");
	if (!pfnCreateFramebuffer) {
		fprintf(stderr, "%s\n", dlerror());
		return false;
	}

	Display* display;
	Window *win_handle;
	Screen *screen;
	int screenID;
	window->GetHandles(display, win_handle, screen, screenID);
#elif defined (_WIN32)
	HMODULE dllHandle = LoadLibrary((library + ".dll").c_str());

	if (!dllHandle) {
		fprintf(stderr, "Failed to load %s!\n", (library + ".dll").c_str());
		return false;
	}

	GraphicsWrapper* (*pfnCreateGraphics)();
	pfnCreateGraphics = (GraphicsWrapper* (*)())GetProcAddress(dllHandle, "createGraphics");

	if (!pfnCreateGraphics) {
		fprintf(stderr, "Cannot get createGraphics function!\n");
		return false;
	}

	pfnCreateVAO = (VertexArrayObject* (*)())GetProcAddress(dllHandle, "createVAO");
	if (!pfnCreateVAO) {
		fprintf(stderr, "Cannot get createVAO function!\n");
		return false;
	}

	pfnCreateVBO = (VertexBufferObject* (*)())GetProcAddress(dllHandle, "createVBO");
	if (!pfnCreateVBO) {
		fprintf(stderr, "Cannot get createVBO function!\n");
		return false;
	}

	pfnCreateShader = (ShaderProgram* (*)())GetProcAddress(dllHandle, "createShader");
	if (!pfnCreateShader) {
		fprintf(stderr, "Cannot get createShader function!\n");
		return false;
	}

	pfnCreateTexture = (Texture* (*)())GetProcAddress(dllHandle, "createTexture");
	if (!pfnCreateTexture) {
		fprintf(stderr, "Cannot get createTexture function!\n");
		return false;
	}

	pfnCreateFramebuffer = (Framebuffer* (*)())GetProcAddress(dllHandle, "createFramebuffer");
	if (!pfnCreateFramebuffer) {
		fprintf(stderr, "Cannot get createFramebuffer function!\n");
		return false;
	}

	HWND win_handle = window->GetHandle();
#endif

	graphicsWrapper = (GraphicsWrapper*)pfnCreateGraphics();
#if defined (__linux__)
	graphicsWrapper->SetWindowContext(display, win_handle, screen, screenID);
#elif defined (_WIN32)
	graphicsWrapper->SetWindowContext(win_handle);
#endif

	if (!graphicsWrapper->InitializeWindowContext())
		return false;
	
	if (!graphicsWrapper->InitializeGraphics())
		return false;

	graphicsWrapper->SetResolution(0, 0, settings.resolutionX, settings.resolutionY);
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

void Engine::Render(glm::mat4 projection, glm::mat4 view, glm::vec2 res) {
	lightSystem.DrawShadows();
	renderPath->Draw(projection, view, player->GetPosition(), res);
#ifdef _WIN32
	graphicsWrapper->SwapBuffer();
#else
	window->SwapBuffer();
#endif
}

void Engine::PlayEngineSound(double sound) {
	audioSystem->Play(0);
}

void Engine::PlayEngineSound2(double sound) {
	audioSystem->Play(1);
}

void Engine::Run() {
	//model = glm::rotate(model, 0.0f, glm::vec3(0));

	float aspectRatio = ((float)settings.resolutionX) / ((float)settings.resolutionY);
	window->ResetCursor();

	//double lag = 0.0;
	//double MS_PER_UPDATE = GetUpdateTimeDelta();

	while (isRunning) {
		CalculateTime();
		//lag += GetRenderTimeDelta();
		window->HandleEvents();
		inputSystem.LoopControls();
		if (lightSystem.directionalLights.size() > 0) {
			unsigned int eID = lightSystem.directionalLights[0].entityID;
			float time = GetTimeCurrent();
			entities[eID].position = glm::vec3(0, glm::sin(time / 4.0f), glm::cos(time / 4.0f)) * 40.0f;
			float ang = std::fmod(time, 360);
			entities[eID].angles = glm::vec3(-3.14159f / 2, 0, 0);
		}
		/*while (lag >= MS_PER_UPDATE)
		{
			lag -= MS_PER_UPDATE;
		}*/
		glm::mat4 projection = glm::perspective(settings.fov, aspectRatio, 0.1f, 100.0f);
		glm::mat4 view = glm::lookAt(
			player->GetPosition(),
			player->GetPosition() + player->GetForward(),
			player->GetUp()
		);

		Render(projection, view, glm::vec2(settings.resolutionX, settings.resolutionY));
	}
}

// Find available path from include paths
std::string Engine::GetAvailablePath(std::string szString) {
	// Check Mods Directory
	// Check Total Conversion Directory
	// Check Game Directory
	// Check Engine Directory
	if (FileExists(szString))
		return szString;

	// Return Empty String
	return "";
}

// Initialize and Load a game scene
bool Engine::InitializeScene(std::string szScenePath) {
	szScenePath = GetAvailablePath(szScenePath);
	if (szScenePath == "") {
		printf("Scene path %s not found.\n", szScenePath.c_str());
		return false;
	}

	player = new EBasePlayer();
	player->position.y = 2;
	// Eventually do all spawning after all initializing is complete.
	player->Spawn();

	// Battletoads/Battletoad_posed.obj
	// crytek-sponza/sponza.obj
	/*
	entities.push_back(EBase());
	geometryCache.LoadModel3D("../models/materialTest/materialTest.obj", entities.size() - 1, entities.back().components[COMPONENT_MODEL], entities.back().components[COMPONENT_RENDER]);

	entities.back().position = glm::vec3(0.0f, 0.5f, 0.0f);
	entities.back().scale = glm::vec3(0.4f, 0.4f, 0.4f);
	entities.push_back(EBase());
	geometryCache.LoadModel3D("../models/Battletoads/Battletoad_posed.obj", entities.size() - 1, entities.back().components[COMPONENT_MODEL], entities.back().components[COMPONENT_RENDER]);
	entities.back().position = glm::vec3(2.0f, 0.0f, 0.0f);
	entities.back().scale = glm::vec3(0.1f, 0.1f, 0.1f);
	entities.push_back(EBase());
	geometryCache.LoadModel3D("../models/Battletoads/Battletoad_posed.obj", entities.size() - 1, entities.back().components[COMPONENT_MODEL], entities.back().components[COMPONENT_RENDER]);
	entities.back().position = glm::vec3(-2.0f, 0.0f, 0.0f);
	entities.back().scale = glm::vec3(0.1f, 0.1f, 0.1f);*/

	entities.push_back(EBase());
	entities.back().position = glm::vec3(-10, 1.5, -4.5);
	entities.back().angles = glm::vec3(0, 3.14159f / 4, 0);
	lightSystem.AddSpotLight((unsigned int)entities.size() - 1, glm::vec3(1, 0.5, 0.5), 400.0f, true, 16, 45, 89);

	entities.push_back(EBase());
	entities.back().position = glm::vec3( 10, 1.5, -4.5);
	entities.back().angles = glm::vec3(-3.14159f / 2, 0, 0);
	lightSystem.AddSpotLight((unsigned int)entities.size() - 1, glm::vec3(1, 1, 1), 10.0f, false, 16, 20, 45);
	
	entities.push_back(EBase());
	entities.back().position = glm::vec3(-10, 1.5, 4.5);
	entities.back().angles = glm::vec3(0, -3.14159f / 2, 0);
	lightSystem.AddSpotLight((unsigned int)entities.size() - 1, glm::vec3(1, 1, 1), 15.0f, true, 16, 40, 80);
	entities.push_back(EBase());
	entities.back().position = glm::vec3( 10, 1.5, 4.5);
	entities.back().angles = glm::vec3(-3.14159f / 2, 0, 0);
	lightSystem.AddSpotLight((unsigned int)entities.size() - 1, glm::vec3(1, 1, 1), 5.0f, false, 16, 30, 60);

	entities.push_back(EBase());
	entities.back().position = glm::vec3(0, 1.5, -4.5);
	lightSystem.AddPointLight((unsigned int)entities.size() - 1, glm::vec3(0.5, 1, 0.5), 40.0f, true, 16);

	entities.push_back(EBase());
	entities.back().position = glm::vec3(-10, 1.5, 0);
	lightSystem.AddPointLight((unsigned int)entities.size() - 1, glm::vec3(1, 1, 1), 30.0f, true, 16);
	
	entities.push_back(EBase());
	entities.back().position = glm::vec3(10, 1.5, 0);
	lightSystem.AddPointLight((unsigned int)entities.size() - 1, glm::vec3(0.5, 1, 0.5), 25.0f, true, 16);
	entities.push_back(EBase());
	entities.back().position = glm::vec3(0, 1.5, 4.5);
	lightSystem.AddPointLight((unsigned int)entities.size() - 1, glm::vec3(1, 1, 1), 22.0f, true, 16);

	entities.push_back(EBase());
	entities.back().position = glm::vec3(0, 1.5, 0);
	lightSystem.AddPointLight((unsigned int)entities.size() - 1, glm::vec3(1, 0.5, 0.5), 100, true, 4);

	entities.push_back(EBase());
	entities.back().position = glm::vec3(10, 1.5, 4.5);
	lightSystem.AddDirectionalLight((unsigned int)entities.size() - 1, glm::vec3(1, 1, 1), 200.0f, true, 32.0f);

	engine.entities.push_back(EBase());
	engine.geometryCache.LoadModel3D("../models/crytek-sponza/sponza.obj", engine.entities.size() - 1, engine.entities.back().components[COMPONENT_MODEL], engine.entities.back().components[COMPONENT_RENDER]);
	engine.entities.back().scale = glm::vec3(0.01f, 0.01f, 0.01f);

	for (int i=-1; i <= 1; i++)
		for (int j=-1; j <= 1; j++)
			cubemapSystem.AddCubemap(glm::vec3(i*10, 1.5, j * 4.5));

	return true;
}

void Engine::CalculateTime() {
	currentTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - prevTime);
	prevTime = currentTime;
}

double Engine::GetTimeCurrent() {
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - startTime).count()/1000000000.0;
}

double Engine::GetUpdateTimeDelta() {
	return (double)deltaTime.count() / 1000000000.0;
}

double Engine::GetRenderTimeDelta() {
	return (double)deltaTime.count() / 1000000000.0;
}

void Engine::Shutdown() {
	isRunning = false;
}

void Engine::ShutdownControl(double) {
	Shutdown();
}

Engine::~Engine() {
	audioSystem->Shutdown();
	window->Shutdown();
}