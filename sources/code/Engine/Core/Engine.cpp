#include "Engine.h"
#include "Utilities.h"
#include "GraphicsDLLPointer.h"
#include "iniHandler.h"
#include <stdio.h>
#include "LevelLoader.h"
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
void				(*pfnDeleteGraphicsPointer)(void *ptr);

bool Engine::Initialize() {
	//physics.Initialize();
	//return false;
	srand((unsigned int)time(NULL));

	// Get Settings here:
	InitializeSettings();
	if (!InitializeWindow())						return false;
	//if (!InitializeAudio())							return false;
	if (!InitializeGraphics(GRAPHICS_OPENGL))		return false;

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

	shader->BindAttribLocation(0, "vertexPos");
	shader->BindAttribLocation(1, "TexCoord");
	shader->BindAttribLocation(2, "vertexNormal");
	shader->BindAttribLocation(3, "vertexTangent");

	shader->BindOutputLocation(0, "position");
	shader->BindOutputLocation(1, "normal");
	shader->BindOutputLocation(2, "albedo");
	shader->BindOutputLocation(3, "specular");
	if (!shader->Compile())
		fprintf(stderr, "Failed to compile main metalness shader %s.\n", vsPath.c_str());

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

	renderPathType = RENDERPATH_DEFERRED;
	switch (renderPathType) {
	default:
		renderPath = (RenderPath *)new RenderPathForward(graphicsWrapper, &geometryCache);
		break;
	case RENDERPATH_DEFERRED:
		renderPath = (RenderPath *)new RenderPathDeferred(graphicsWrapper, &geometryCache, &terrainSystem);
		break;
	};

	lightSystem.SetPointers(graphicsWrapper, &geometryCache);

	inputSystem.AddControl("escape", "Shutdown", NULL, 1);
	inputSystem.BindAction("Shutdown", NULL, this, &Engine::ShutdownControl, KEY_RELEASED);

	inputSystem.AddControl("e", "PlaySound", NULL, 1);
	inputSystem.BindAction("PlaySound", NULL, this, &Engine::PlayEngineSound);

	inputSystem.AddControl("r", "PlaySound2", NULL, 1);
	inputSystem.BindAction("PlaySound2", NULL, this, &Engine::PlayEngineSound2);

	inputSystem.AddControl("q", "CaptureCubemaps", NULL, 1);
	inputSystem.BindAction("CaptureCubemaps", NULL, &(engine.cubemapSystem), &CubemapSystem::CaptureCubemaps);

	inputSystem.AddControl("1", "SwitchDebug", NULL, 1);
	inputSystem.BindAction("SwitchDebug", NULL, this, &Engine::SwitchDebug);

	terrainSystem.Initialize();
	if (!InitializeScene(defaultMap))	return false;
	
	if (settings.enableReflections)
		cubemapSystem.LoadCubemaps();

	isRunning = true;
	prevTime = std::chrono::high_resolution_clock::now();
	startTime = std::chrono::high_resolution_clock::now();
	printf("Initialization Complete! Starting:\n==================================\n");
	return true;
}

/*void Engine::AddSystem(System *newSystem) {
	systems.push_back(newSystem);
}

void Engine::AddSpace(Space *newSpace) {
	space.push_back(newSpace);
}*/

void Engine::InitializeSettings() {
	INIConfigFile cfile;
	
	if (cfile.Initialize("../settings.ini")) {
		cfile.GetInteger("Window", "resx",	1366,	settings.resolutionX);
		cfile.GetInteger("Window", "resy",	768,	settings.resolutionY);
		cfile.GetFloat(  "Window", "fov",	90,		settings.fov);
		settings.fov *= 3.14159f / 360.0f; // Convert to rad, /2 for full fovY.
		std::string graphics;
		cfile.GetString("Renderer", "graphics", "OpenGL", graphics);
		cfile.GetBool("Renderer", "reflections", true, settings.enableReflections);
		cfile.GetBool("Renderer", "shadows", true, settings.enableShadows);
		cfile.GetString("Game", "defaultmap", "../scenes/terrain.json", defaultMap);

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

		cfile.SaveFile();
	}
	else {
		fprintf(stderr, "SETTINGS.INI: File not found.\n");

		cfile.SetInteger("Window", "resx", 1366);
		cfile.SetInteger("Window", "resy", 768);
		cfile.SetFloat("Window", "fov", 90);
		cfile.SetString("Renderer", "graphics", "OpenGL");
		cfile.SetBool("Renderer", "reflections", true);
		cfile.SetBool("Renderer", "shadows", true);
		cfile.SetString("Game", "defaultmap", "../scenes/terrain.json");

		settings.resolutionX = 1366;
		settings.resolutionY = 768;
		settings.graphicsLanguage = GRAPHICS_OPENGL;
		settings.fov = 90;
		settings.fov *= 3.14159f / 360.0f; // Convert to rad, /2 for full fovY.
		settings.enableReflections = true;
		settings.enableShadows = false;
		defaultMap = "../scenes/terrain.json";
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
	window->SetInputPointer(&inputSystem);
	if (!window->Initialize("The Grindstone Engine", settings.resolutionX, settings.resolutionY))
		return false;

	window->SetCursorShown(false);

	return true;
}

bool Engine::InitializeAudio() {
#if defined (__linux__)
	void *lib_handle = dlopen("./audiosdl.so", RTLD_LAZY);

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
	HMODULE dllHandle = LoadLibrary("audiosdl.dll");

	if (!dllHandle) {
		fprintf(stderr, "Failed to load audiosdl.dll!\n");
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
	sounds.push_back(audioSystem->LoadSound("../sounds/snaredrum.wav"));
	sounds.push_back(audioSystem->LoadSound("../sounds/kickdrum.wav"));

	return true;
}

bool Engine::InitializeGraphics(GraphicsLanguage gl) {
	std::string library;
	switch (gl) {
	default:
		library = "graphicsgl";
		break;
#ifndef __APPLE__
	case GRAPHICS_VULKAN:
		library = "graphicsvk";
		break;
#endif
#ifdef _WIN32
	case GRAPHICS_DIRECTX:
		library = "graphicsdx";
		break;
#endif
#ifdef __APPLE__
	case GRAPHICS_METAL:
		library = "graphicsml";
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

	pfnDeleteGraphicsPointer = (void (*)(void*))dlsym(lib_handle, "deletePointer");
	if (!pfnDeleteGraphicsPointer) {
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

	pfnDeleteGraphicsPointer = (void (*)(void *))GetProcAddress(dllHandle, "deletePointer");
	if (!pfnDeleteGraphicsPointer) {
		fprintf(stderr, "Cannot get deletePointer graphics function!\n");
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

void Engine::Render(glm::mat4 projection, glm::mat4 view, bool usePost) {
	renderPath->Draw(projection, view, player->GetPosition(), settings.enableReflections && usePost);
#ifdef _WIN32
	graphicsWrapper->SwapBuffer();
#else
	window->SwapBuffer();
#endif
}

void Engine::PlayEngineSound(double sound) {
	int v = rand() % sounds.size();
	if (v < sounds.size())
		sounds[v]->Play();
}

void Engine::PlayEngineSound2(double sound) {
	sounds[0]->Stop();
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

		glm::mat4 projection = glm::perspective(settings.fov, aspectRatio, 0.1f, 100.0f);
		glm::mat4 view = glm::lookAt(
			player->GetPosition(),
			player->GetPosition() + player->GetForward(),
			player->GetUp()
		);

		if (settings.enableShadows)
			lightSystem.DrawShadows();
		graphicsWrapper->SetResolution(0, 0, settings.resolutionX, settings.resolutionY);
		Render(projection, view, true);
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

void Engine::SwitchDebug(double) {
	debugMode++;
	if (debugMode == NUM_DEBUG)
		debugMode = DEBUG_NONE;
}

EBase *Engine::createEntity(const char * szEntityName) {
	for (size_t i = 0; i < classRegistry.size(); i++) {
		if (szEntityName == classRegistry[i]->entityName)
			if (classRegistry[i]->function)
				return (EBase *)classRegistry[i]->function();
	}
	return nullptr;
}

void Engine::registerClass(const char * szEntityName, std::function<void*()> fn) {
	classRegistry.push_back(new classRegister(szEntityName, fn));
}

// Initialize and Load a game scene
bool Engine::InitializeScene(std::string szScenePath) {
	std::string szSceneNewPath = GetAvailablePath(szScenePath);

	if (szSceneNewPath == "") {
		printf("Scene path %s not found.\n", szScenePath.c_str());
		return false;
	}

	LoadLevel(szSceneNewPath);
	geometryCache.LoadPreloaded();
	terrainSystem.GenerateComponents();

	player = new EBasePlayer();
	player->position.y = 2;
	player->Spawn();

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
	geometryCache.Shutdown();

	if (audioSystem)
		audioSystem->Shutdown();
	
	if (window)
		window->Shutdown();
}