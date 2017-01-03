#include "Engine.h"
#include "Utilities.h"
#include "GraphicsDLLPointer.h"
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
	// Get Settings here:
	settings.resolutionX = 1024;
	settings.resolutionY = 768;

	if (!InitializeWindow())					return false;
	if (!InitializeGraphics())					return false;
	if (!InitializeScene("../scenes/startup.gmf"))	return false;


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
		return false;

	std::string fsContent;
	if (!ReadFile(fsPath, fsContent))
		return false;
	
	shader = pfnCreateShader();
	shader->AddShader(&vsPath, &vsContent, SHADER_VERTEX);
	shader->AddShader(&fsPath, &fsContent, SHADER_FRAGMENT);
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

	renderPathType = RENDERPATH_DEFERRED;
	switch (renderPathType) {
	default:
		renderPath = (RenderPath *)new RenderPathForward(graphicsWrapper, &geometryCache);
		break;
	case RENDERPATH_DEFERRED:
		renderPath = (RenderPath *)new RenderPathDeferred(graphicsWrapper, &geometryCache);
		break;
	};

	engine.inputSystem.AddControl("escape", "Shutdown", NULL, 1);
	engine.inputSystem.BindAction("Shutdown", NULL, &engine, &Engine::ShutdownControl);

	isRunning = true;
	prevTime = std::chrono::high_resolution_clock::now();
	startTime = std::chrono::high_resolution_clock::now();
	return true;
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
	if (!window->Initialize("The Grind Engine", 1024, 768))
		return false;

	window->SetInputPointer(&inputSystem);
	window->SetCursorShown(false);

	return true;
}

bool Engine::InitializeGraphics() {
#if defined (__linux__)
	void *lib_handle = dlopen("./opengl.so", RTLD_LAZY);

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
	std::cout << "Getting Handles\n";
	window->GetHandles(display, win_handle, screen, screenID);
	std::cout << "Handles gotten\n";
#elif defined (_WIN32)
	HMODULE dllHandle = LoadLibrary("opengl.dll");

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

	std::cout << "Creating GraphicsWrapper\n";
	std::cout << pfnCreateGraphics << "\n";
	graphicsWrapper = (GraphicsWrapper*)pfnCreateGraphics();
#if defined (__linux__)
	std::cout << "Passing Context\n";
	graphicsWrapper->SetWindowContext(display, win_handle, screen, screenID);
	std::cout << "Context Passed\n";
#elif defined (_WIN32)
	std::cout << "Passing Context\n";
	graphicsWrapper->SetWindowContext(win_handle);
	std::cout << "Context Passed\n";
#endif

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

struct UniformBuffer {
	glm::mat4 pvmMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	int texLoc0;
	int texLoc1;
	int texLoc2;
	int texLoc3;
} ubo;

void Engine::Run() {
	glm::mat4x4 model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6, 0, 0));
	model = glm::scale(model, glm::vec3(0.01f));
	//model = glm::rotate(model, 0.0f, glm::vec3(0));

	glm::mat4 projection = glm::perspective( 45.0f, 4.0f / 3.0f, 0.1f, 100.0f );

	window->ResetCursor();

	ubo.texLoc0 = 0;
	ubo.texLoc1 = 1;
	ubo.texLoc2 = 2;
	ubo.texLoc3 = 3;

	while (isRunning) {
		CalculateTime();
		window->HandleEvents();
		inputSystem.LoopControls();

		glm::mat4 view = glm::lookAt(
			player->getPosition(),
			player->getPosition() + player->getForward(),
			player->getUp()
		);

		shader->Use();
		ubo.pvmMatrix = projection * view * model;
		ubo.viewMatrix = view;
		ubo.modelMatrix = model;
		shader->PassData(&ubo);
		shader->SetUniform4m();
		shader->SetUniform4m();
		shader->SetUniform4m();
		shader->SetInteger();
		shader->SetInteger();
		shader->SetInteger();
		shader->SetInteger();

		renderPath->Draw(player->getPosition());
#ifdef _WIN32
		graphicsWrapper->SwapBuffer();
#else
		window->SwapBuffer();
#endif
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

	// Eventually do all spawning after all initializing is complete.
	player->Spawn();

	geometryCache.LoadModel3D("../models/crytek-sponza/sponza.obj");

	return true;
}

void Engine::CalculateTime() {
	currentTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevTime);
	prevTime = currentTime;
}

double Engine::GetTimeCurrent() {
	return (double)std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count()/1000.0;
}

double Engine::GetTimeDelta() {
	return (double)deltaTime.count() / 1000.0;
}

void Engine::Shutdown() {
	isRunning = false;
}

void Engine::ShutdownControl(double) {
	Shutdown();
}

Engine::~Engine() {
	window->Shutdown();
}