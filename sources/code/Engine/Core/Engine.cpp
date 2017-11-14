#include "Engine.h"
#include "Utilities.h"
#include "iniHandler.h"
#include <stdio.h>
#include "LevelLoader.h"
#include "Systems/SGeometryStatic.h"

#if defined(_WIN32)
	#define LoadDLL(path) HMODULE dllHandle = LoadLibrary((path+".dll").c_str()); \
	if (!dllHandle) { \
		fprintf(stderr, "Failed to load %s!\n", path.c_str()); \
		return false; \
	}

	#define LoadDLLFunction(string) GetProcAddress(dllHandle, string);
#elif defined(__linux__)
	#include <dlfcn.h>

	#define LoadDLL(path) void *lib_handle = dlopen(("./"+path+".so").c_str(), RTLD_LAZY);\
	if (!lib_handle) {\
		fprintf(stderr, "Failed to load %s: %s\n", path.c_str(), dlerror());\
		return false;\
	}

	#define LoadDLLFunction(string) dlsym(lib_handle, string);
#endif

#ifdef UseClassInstance
	Engine *Engine::=0;
#endif

bool Engine::Initialize() {
	srand((unsigned int)time(NULL));

	// Get Settings here:
	InitializeSettings();
	if (!InitializeGraphics(engine.settings.graphicsLanguage))		return false;
	physicsSystem.Initialize();

	lightSystem.SetPointers(graphics_wrapper_, &geometry_system);

	renderPathType = RENDERPATH_DEFERRED;
	switch (renderPathType) {
	default:
		//renderPath = (RenderPath *)new RenderPathForward(graphics_wrapper_, &geometry_system);
		break;
	case RENDERPATH_DEFERRED:
		renderPath = (RenderPath *)new RenderPathDeferred(graphics_wrapper_);
		break;
	};

	CheckModPaths();

	//sUi.LoadDocument("test.rml");

	inputSystem.AddControl("escape", "Shutdown", NULL, 1);
	inputSystem.BindAction("Shutdown", NULL, this, &Engine::ShutdownControl, KEY_RELEASED);

	inputSystem.AddControl("q", "CaptureCubemaps", NULL, 1);
	inputSystem.BindAction("CaptureCubemaps", NULL, &(engine.cubemapSystem), &CubemapSystem::CaptureCubemaps);

	inputSystem.AddControl("1", "SwitchDebug", NULL, 1);
	inputSystem.BindAction("SwitchDebug", NULL, this, &Engine::SwitchDebug);

	//terrainSystem.Initialize();
	if (!InitializeScene(defaultMap))	return false;

	cameraSystem.components[0].SetAspectRatio((float)engine.settings.resolutionX/engine.settings.resolutionY);
	
	if (settings.enableReflections)
		cubemapSystem.LoadCubemaps();

	isRunning = true;
	prevTime = std::chrono::high_resolution_clock::now();
	startTime = std::chrono::high_resolution_clock::now();
	printf("Initialization Complete! Starting:\n==================================\n");
	return true;
}

void Engine::InitializeSettings() {
	INIConfigFile cfile;
	
	if (cfile.Initialize("../settings.ini")) {
		cfile.GetBool("Window", "vsync", true, settings.vsync);
		cfile.GetInteger("Window", "resx",	1366,	settings.resolutionX);
		cfile.GetInteger("Window", "resy",	768,	settings.resolutionY);
		cfile.GetFloat(  "Window", "fov",	90,		settings.fov);
		settings.fov *= 3.14159f / 360.0f; // Convert to rad, /2 for full fovY.
		std::string graphics;
		cfile.GetString("Renderer", "graphics", "OpenGL", graphics);
		cfile.GetBool("Renderer", "reflections", true, settings.enableReflections);
		cfile.GetBool("Renderer", "shadows", true, settings.enableShadows);
		cfile.GetBool("Renderer", "debugNoLighting", false, settings.debugNoLighting);
		cfile.GetString("Game", "defaultmap", "../assets/scenes/sponza.json", defaultMap);

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

		cfile.SetBool("Window", "vsync", true);
		cfile.SetInteger("Window", "resx", 1366);
		cfile.SetInteger("Window", "resy", 768);
		cfile.SetFloat("Window", "fov", 90);
		cfile.SetString("Renderer", "graphics", "OpenGL");
		cfile.SetBool("Renderer", "reflections", true);
		cfile.SetBool("Renderer", "shadows", true);
		cfile.SetBool("Renderer", "debugNoLighting", false);
		cfile.SetString("Game", "defaultmap", "../assets/scenes/sponza.json");

		settings.resolutionX = 1366;
		settings.resolutionY = 768;
		settings.graphicsLanguage = GRAPHICS_OPENGL;
		settings.fov = 90.0f * (3.14159f / 360.0f); // Convert to rad, /2 for full fovY.
		settings.enableReflections = true;
		settings.enableShadows = false;
		settings.debugNoLighting = false;
		settings.vsync = true;
		defaultMap = "../assets/scenes/sponza.json";
	}

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
	
	LoadDLL(library);

	GraphicsWrapper* (*pfnCreateGraphics)(InstanceCreateInfo) = (GraphicsWrapper* (*)(InstanceCreateInfo))LoadDLLFunction("createGraphics");
	if (!pfnCreateGraphics) {
		fprintf(stderr, "Cannot get createGraphics function!\n");
		return false;
	}

	void (*pfnDeleteGraphics)(void*) = (void (*)(void*))LoadDLLFunction("deleteGraphics");
	if (!pfnDeleteGraphics) {
		fprintf(stderr, "Cannot get deleteGraphics function!\n");
		return false;
	}

	InstanceCreateInfo createInfo;
	createInfo.width = engine.settings.resolutionX;
	createInfo.height = engine.settings.resolutionY;
	createInfo.vsync = engine.settings.vsync;
	createInfo.inputInterface = &inputSystem;
	createInfo.title = "Grindstone";
#ifdef NDEBUG
	createInfo.debug = false;
#else
	createInfo.debug = true;
#endif
	graphics_wrapper_ = (GraphicsWrapper*)pfnCreateGraphics(createInfo);


	graphics_wrapper_->CreateDefaultStructures();

	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "UniformBufferObject";
	ubbci.size = sizeof(myUBO);
	ubbci.stages = SHADER_STAGE_VERTEX_BIT;
	UniformBufferBinding *ubb = graphics_wrapper_->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(MatUniformBufferObject);
	ubci.binding = ubb;
	ubo = graphics_wrapper_->CreateUniformBuffer(ubci);

	UniformBufferBindingCreateInfo ubbci2;
	ubbci2.binding = 1;
	ubbci2.shaderLocation = "ModelMatrixBuffer";
	ubbci2.size = sizeof(modelUBO);
	ubbci2.stages = SHADER_STAGE_VERTEX_BIT;
	UniformBufferBinding *ubb2 = graphics_wrapper_->CreateUniformBufferBinding(ubbci2);

	UniformBufferCreateInfo ubci2;
	ubci2.isDynamic = false;
	ubci2.size = sizeof(modelUBO);
	ubci2.binding = ubb2;
	ubo2 = graphics_wrapper_->CreateUniformBuffer(ubci2);

	materialManager.Initialize(graphics_wrapper_, vbd, vads, ubb);
	geometry_system.AddSystem(new SGeometryStatic(graphics_wrapper_));

	std::vector<ColorFormat> gbufferCFs = {
		FORMAT_COLOR_R8G8B8A8,	// R  G  B  MatID
		FORMAT_COLOR_R8G8B8A8,	// sR sG sB Roughness
		FORMAT_COLOR_R8G8B8A8	// nX nY nZ
	};
	
	FramebufferCreateInfo gbufferCI;
	gbufferCI.colorFormats = gbufferCFs.data();
	gbufferCI.depthFormat = FORMAT_DEPTH_24;
	gbufferCI.numColorTargets = (uint32_t)gbufferCFs.size();
	gbufferCI.width = engine.settings.resolutionX;
	gbufferCI.height = engine.settings.resolutionY;
	gbufferCI.renderPass = nullptr;
	gbuffer = graphics_wrapper_->CreateFramebuffer(gbufferCI);

	ColorFormat deviceColorFormat = graphics_wrapper_->GetDeviceColorFormat();
	FramebufferCreateInfo defaultFramebufferCI;
	defaultFramebufferCI.colorFormats = &deviceColorFormat;
	defaultFramebufferCI.depthFormat = FORMAT_DEPTH_24;
	defaultFramebufferCI.numColorTargets = 1;
	defaultFramebufferCI.width = engine.settings.resolutionX;
	defaultFramebufferCI.height = engine.settings.resolutionY;
	defaultFramebufferCI.renderPass = nullptr;
	defaultFramebuffer = graphics_wrapper_->CreateFramebuffer(defaultFramebufferCI);

	return true;
}

Engine &Engine::GetInstance() {
	// Create the Engine instance when "GetInstance()" is called (ie: when "engine" is used).
	static Engine newEngine;
	return newEngine;
}

void Engine::Render(glm::mat4 _projMat, glm::mat4 _viewMat) {
	if (graphics_wrapper_->SupportsCommandBuffers()) {
		materialManager.DrawDeferred();
		graphics_wrapper_->WaitUntilIdle();
	}
	else {
		if (!engine.settings.debugNoLighting) {
			gbuffer->BindWrite();
			gbuffer->Clear();
			materialManager.DrawImmediate();
			gbuffer->Unbind();
			graphics_wrapper_->BindDefaultFramebuffer();
			gbuffer->BindRead();
			renderPath->Draw(gbuffer);
			gbuffer->Unbind();

			graphics_wrapper_->SwapBuffer();
		}
		else {
			graphics_wrapper_->BindDefaultFramebuffer();
			graphics_wrapper_->Clear();
			materialManager.DrawImmediate();
			//graphics_wrapper_->Blit(0,0,0,1366,768);
			graphics_wrapper_->SwapBuffer();
		}
	}
}

void Engine::Run() {
	graphics_wrapper_->ResetCursor();

	while (isRunning) {
		CalculateTime();

		graphics_wrapper_->HandleEvents();
		inputSystem.LoopControls();

		gameplay_system.Update(GetUpdateTimeDelta());
		physicsSystem.StepSimulation(GetUpdateTimeDelta());

		if (cameraSystem.components.size() > 0) {
			CCamera *cam = &cameraSystem.components[0];
			
			myUBO.proj = cam->GetProjection();
			myUBO.view = cam->GetView();

			modelUBO.model = glm::scale(glm::vec3(0.01f, 0.01f, 0.01f));
			ubo2->UpdateUniformBuffer(&modelUBO);
			ubo2->Bind();
			ubo->UpdateUniformBuffer(&myUBO);

			if (settings.enableShadows)
				lightSystem.DrawShadows();
			ubo->Bind();
			Render(myUBO.proj, myUBO.view);
		}

		//sUi.Update();
		//sUi.Render();
	}
}

void Engine::CheckModPaths() {
	std::ifstream file;
	file.open("/mods/activemods.txt");

	if (!file.fail()) {
		std::string line;
		while (std::getline(file, line)) {
#ifdef __APPLE__
			modPaths.push_back(getResourcePath()+line);
#else
			modPaths.push_back(line);
#endif
		}
	}
}

// Find available path from include paths
std::string Engine::GetAvailablePath(std::string szString) {
	// Check Mods Directory
	for (int i = 0; i < modPaths.size(); i++) {
		std::string modPath = modPaths[i] + szString;
		FileExists(modPaths[i] + szString);
		return modPath;
	}

#ifdef __APPLE__
	if (FileExists(szString))
		return getResourcePath()+szString;
#else
	if (FileExists(szString))
		return szString;
#endif

	// Return Empty String
	return "";
}

void Engine::SwitchDebug(double) {
	debugMode++;
	if (debugMode == NUM_DEBUG)
		debugMode = DEBUG_NONE;
}

// Initialize and Load a game scene
bool Engine::InitializeScene(std::string szScenePath) {
	std::string szSceneNewPath = GetAvailablePath(szScenePath);

	if (szSceneNewPath == "") {
		printf("Scene path %s not found.\n", szScenePath.c_str());
		return false;
	}

	LoadLevel(szSceneNewPath);
	geometry_system.LoadPreloaded();
	materialManager.LoadPreloaded;

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
	physicsSystem.Cleanup();

	if (gbuffer)
		graphics_wrapper_->DeleteFramebuffer(gbuffer);

	if (defaultFramebuffer)
		graphics_wrapper_->DeleteFramebuffer(defaultFramebuffer);

	if (graphics_wrapper_)
		graphics_wrapper_->Cleanup();
}