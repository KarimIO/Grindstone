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

bool Engine::Initialize() {
	if (!InitializeWindow())					return false;
	if (!InitializeGraphics())					return false;
	if (!InitializeScene("scenes/startup.gmf"))	return false;


	// An array of 3 vectors which represents 3 vertices
	float g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};

	std::string vsPath = "shaders/objects/main.glvs"; // GetShaderExt()
	std::string fsPath = "shaders/objects/main.glfs";

	std::string vsContent;
	if (!ReadFile(vsPath, vsContent))
		return false;

	std::string fsContent;
	if (!ReadFile(fsPath, fsContent))
		return false;
	
	shader = pfnCreateShader();
	shader->AddShader(vsPath, vsContent, SHADER_VERTEX);
	shader->AddShader(fsPath, fsContent, SHADER_FRAGMENT);
	shader->Compile();

	shader->SetNumUniforms(4);
	shader->CreateUniform("pvmMatrix");
	shader->CreateUniform("modelMatrix");
	shader->CreateUniform("viewMatrix");
	shader->CreateUniform("tex");

	vsContent.clear();
	fsContent.clear();

	vao = pfnCreateVAO();
	vao->Initialize();
	vao->Bind();

	vbo = pfnCreateVBO();
	vbo->Initialize(1);
	vbo->AddVBO(g_vertex_buffer_data, sizeof(g_vertex_buffer_data), 3, SIZE_FLOAT, DRAW_STATIC);
	vbo->Bind(0);

	geometryCache.LoadModel3D("models/crytek-sponza/sponza.obj");

	isRunning = true;
	prevTime = std::chrono::high_resolution_clock::now();
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

struct UniformBuffer {
	glm::mat4 pvmMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	int textureLocation;
} ubo;

void Engine::Run() {

	shader->Use();

	glm::mat4x4 model = glm::mat4(1);
	//model = glm::translate(model,glm::vec3(0,0,0));
	model = glm::scale(model, glm::vec3(0.01f));
	//model = glm::rotate(model, 0.0f, glm::vec3(0));

	glm::mat4 projection = glm::perspective(
		45.0f,         // The horizontal Field of View, in degrees : the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
		4.0f / 3.0f, // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
		0.1f,        // Near clipping plane. Keep as big as possible, or you'll get precision issues.
		100.0f       // Far clipping plane. Keep as little as possible.
		);

	window->ResetCursor();

	ubo.textureLocation = 0;

	position = glm::vec3(0, 1, -1);

	while (isRunning) {
		CalculateTime();

		glm::mat4 view = glm::lookAt(
			position,				// the position of your camera, in world space
			position+getForward(),	// where you want to look at, in world space
			getUp()					// probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
		);

		ubo.pvmMatrix = projection * view * model;
		ubo.viewMatrix = view;
		ubo.modelMatrix = model;
		shader->PassData(&ubo);
		shader->SetUniform4m();
		shader->SetUniform4m();
		shader->SetUniform4m();
		shader->SetInteger();

		graphicsWrapper->Clear();
		geometryCache.Draw();
		graphicsWrapper->SwapBuffer();

		window->HandleEvents();
		window->ResetCursor();
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

	return true;
}

void Engine::CalculateTime() {
	currentTime = std::chrono::high_resolution_clock::now();
	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - prevTime);
	prevTime = currentTime;
}

double Engine::GetTimeCurrent() {
	return 0;
}

double Engine::GetTimeDelta() {
	return (double)deltaTime.count();
}

void Engine::Shutdown() {
	isRunning = false;
}

Engine::~Engine() {
	window->Shutdown();
}


glm::vec3 Engine::getForward() {
	glm::vec3 ang = angles;
	return glm::vec3(
		glm::cos(ang.x) * glm::sin(ang.y),
		glm::sin(ang.x),
		glm::cos(ang.x) * glm::cos(ang.y)
	);
}

glm::vec3 Engine::getUp() {
	return glm::cross(getRight(), getForward());
}

glm::vec3 Engine::getRight() {
	glm::vec3 ang = angles;
	return glm::vec3(
		glm::sin(ang.y - 3.14159f / 2.0f),
		0,
		glm::cos(ang.y - 3.14159f / 2.0f)
	);
}