#include "Engine.hpp"
#include "Utilities.hpp"
#include "iniHandler.hpp"
#include <stdio.h>
#include "LevelLoader.hpp"
#include "LoadingScreen.hpp"
#include "Systems/SGeometryStatic.hpp"
#include <thread>

#if defined(_WIN32)
	#define LoadDLL(path) HMODULE dllHandle = LoadLibrary((path+".dll").c_str()); \
	if (!dllHandle) { \
		fprintf(stderr, "Failed to load %s!\n", path.c_str()); \
		return false; \
	}

	#define LoadDLLFunction(string) GetProcAddress(dllHandle, string);
#elif defined(__linux__)
	#include <dlfcn.h>

	#define LoadDLL(path) void *lib_handle = dlopen(("./lib"+path+".so").c_str(), RTLD_LAZY);\
	if (!lib_handle) {\
		fprintf(stderr, "Failed to load %s: %s\n", path.c_str(), dlerror());\
		return false;\
	}

	#define LoadDLLFunction(string) dlsym(lib_handle, string);
#endif

#ifdef UseClassInstance
	Engine *Engine::=0;
#endif

bool run_loading = true;
void Engine::LoadingScreenThread() {
	auto start = std::chrono::high_resolution_clock::now();
	LoadingScreen screen(graphics_wrapper_);
	while (run_loading) {
		auto curr = std::chrono::high_resolution_clock::now();
		double t = std::chrono::duration_cast<std::chrono::nanoseconds>(curr - start).count() / 100000000.0f;
		screen.Render(t);
	}
}

#define MULTITHEAD_LOAD 0

bool Engine::Initialize() {
	srand((unsigned int)time(NULL));

	// Get Settings here:
	InitializeSettings();
	if (!InitializeGraphics(settings.graphicsLanguage))		return false;
#if MULTITHEAD_LOAD
	std::thread t1(&Engine::LoadingScreenThread, this);
#else
	run_loading = false;
	LoadingScreenThread();
#endif

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
	inputSystem.BindAction("CaptureCubemaps", NULL, &(cubemapSystem), &CubemapSystem::CaptureCubemaps);

	inputSystem.AddControl("1", "SwitchDebug", NULL, 1);
	inputSystem.BindAction("SwitchDebug", NULL, this, &Engine::SwitchDebug);

	//terrainSystem.Initialize();
	if (!InitializeScene(defaultMap))	return false;

	cameraSystem.components[0].SetAspectRatio((float)settings.resolutionX/settings.resolutionY);
	
	if (settings.enableReflections)
		cubemapSystem.LoadCubemaps();

	isRunning = true;
	prevTime = std::chrono::high_resolution_clock::now();
	startTime = std::chrono::high_resolution_clock::now();
	printf("Initialization Complete! Starting:\n==================================\n");

#if MULTITHEAD_LOAD
	run_loading = false;
	t1.join();
#endif

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
	createInfo.width = settings.resolutionX;
	createInfo.height = settings.resolutionY;
	createInfo.vsync = settings.vsync;
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
	ubbci.size = sizeof(glm::mat4);
	ubbci.stages = SHADER_STAGE_VERTEX_BIT;
	UniformBufferBinding *ubb = graphics_wrapper_->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = true;
	ubci.size = sizeof(glm::mat4);
	ubci.binding = ubb;
	ubo = graphics_wrapper_->CreateUniformBuffer(ubci);

	UniformBufferBindingCreateInfo ubbci2;
	ubbci2.binding = 1;
	ubbci2.shaderLocation = "ModelMatrixBuffer";
	ubbci2.size = sizeof(glm::mat4);
	ubbci2.stages = SHADER_STAGE_VERTEX_BIT;
	UniformBufferBinding *ubb2 = graphics_wrapper_->CreateUniformBufferBinding(ubbci2);
	
	UniformBufferCreateInfo ubci2;
	ubci2.isDynamic = true;
	ubci2.size = sizeof(glm::mat4);
	ubci2.binding = ubb2;
	ubo2 = graphics_wrapper_->CreateUniformBuffer(ubci2);

	std::vector<UniformBufferBinding *> ubbs = { ubb, ubb2 };

	VertexBindingDescription vbd;
	std::vector<VertexAttributeDescription> vads;
	vbd.binding = 0;
	vbd.elementRate = false;
	vbd.stride = sizeof(Vertex);

	vads.resize(4);
	vads[0].binding = 0;
	vads[0].location = 0;
	vads[0].format = VERTEX_R32_G32_B32;
	vads[0].size = 3;
	vads[0].name = "vertexPosition";
	vads[0].offset = offsetof(Vertex, positions);
	vads[0].usage = ATTRIB_POSITION;

	vads[1].binding = 0;
	vads[1].location = 1;
	vads[1].format = VERTEX_R32_G32_B32;
	vads[1].size = 3;
	vads[1].name = "vertexNormal";
	vads[1].offset = offsetof(Vertex, normal);
	vads[1].usage = ATTRIB_NORMAL;

	vads[2].binding = 0;
	vads[2].location = 2;
	vads[2].format = VERTEX_R32_G32_B32;
	vads[2].size = 3;
	vads[2].name = "vertexTangent";
	vads[2].offset = offsetof(Vertex, tangent);
	vads[2].usage = ATTRIB_TANGENT;

	vads[3].binding = 0;
	vads[3].location = 3;
	vads[3].format = VERTEX_R32_G32;
	vads[3].size = 2;
	vads[3].name = "vertexTexCoord";
	vads[3].offset = offsetof(Vertex, texCoord);
	vads[3].usage = ATTRIB_TEXCOORD0;

	planeVBD.binding = 0;
	planeVBD.elementRate = false;
	planeVBD.stride = sizeof(float) * 2;

	planeVAD.binding = 0;
	planeVAD.location = 0;
	planeVAD.format = VERTEX_R32_G32;
	planeVAD.size = 2;
	planeVAD.name = "vertexPosition";
	planeVAD.offset = 0;
	planeVAD.usage = ATTRIB_POSITION;

	materialManager.Initialize(graphics_wrapper_, vbd, vads, ubbs);
	geometry_system.AddSystem(new SGeometryStatic(&materialManager, graphics_wrapper_, vbd, vads));

	std::vector<RenderTargetCreateInfo> gbuffer_images_ci;
	gbuffer_images_ci.reserve(4);
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, settings.resolutionX, settings.resolutionY); // R  G  B matID
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, settings.resolutionX, settings.resolutionY); // nX nY nZ
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, settings.resolutionX, settings.resolutionY); // sR sG sB Roughness
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, settings.resolutionX, settings.resolutionY); // sR sG sB Roughness
	gbuffer_images_ = graphics_wrapper_->CreateRenderTarget(gbuffer_images_ci.data(), gbuffer_images_ci.size());

	RenderTargetCreateInfo depth_image_ci(FORMAT_DEPTH_32, settings.resolutionX, settings.resolutionY);
	depth_image_ = graphics_wrapper_->CreateRenderTarget(&depth_image_ci, 1);
	
	FramebufferCreateInfo gbuffer_ci;
	gbuffer_ci.render_target_lists = &gbuffer_images_;
	gbuffer_ci.num_render_target_lists = 1;
	gbuffer_ci.depth_target = depth_image_;
	gbuffer_ci.render_pass = nullptr;
	gbuffer = graphics_wrapper_->CreateFramebuffer(gbuffer_ci);

	return true;
}

Engine &Engine::GetInstance() {
	// Create the Engine instance when "GetInstance()" is called (ie: when "engine" is used).
	static Engine newEngine;
	return newEngine;
}

void Engine::Render() {
	if (graphics_wrapper_->SupportsCommandBuffers()) {
		materialManager.DrawDeferredCommand();
		graphics_wrapper_->WaitUntilIdle();
	}
	else {
		if (!settings.debugNoLighting) {
			gbuffer->Bind();
			gbuffer->Clear();
			graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
			materialManager.DrawDeferredImmediate();

			graphics_wrapper_->EnableDepth(false);
			graphics_wrapper_->SetImmediateBlending(BLEND_ADDITIVE);
			renderPath->Draw(gbuffer);
			graphics_wrapper_->EnableDepth(true);
			graphics_wrapper_->SetImmediateBlending(BLEND_ADD_ALPHA);
			materialManager.DrawForwardImmediate();

			graphics_wrapper_->BindDefaultFramebuffer();
			gbuffer->BindRead();
			gbuffer->Blit(3, 0, 0, engine.settings.resolutionX, engine.settings.resolutionY);
			graphics_wrapper_->SwapBuffer();
		}
		else {
			graphics_wrapper_->BindDefaultFramebuffer();
			graphics_wrapper_->Clear();
			graphics_wrapper_->SetImmediateBlending(BLEND_NONE);
			materialManager.DrawDeferredImmediate();
			graphics_wrapper_->SetImmediateBlending(BLEND_ADD_ALPHA);
			materialManager.DrawForwardImmediate();
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
		physicsSystem.Update(GetUpdateTimeDelta());
		transformSystem.Update();

		glm::mat4 pv;
		if (cameraSystem.components.size() > 0) {
			CCamera *cam = &cameraSystem.components[0];
			materialManager.resetDraws();
			geometry_system.Cull(cam);

			pv = cam->GetProjection() * cam->GetView();
			ubo->UpdateUniformBuffer(&pv);
			ubo->Bind();
			ubo2->Bind();

			if (settings.enableShadows)
				lightSystem.DrawShadows();
			Render();
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
	materialManager.LoadPreloaded();

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
	std::cout << "Cleaning Physics System...";
	physicsSystem.Cleanup();
	std::cout << "Physics System cleaned.";

	if (gbuffer) {
		std::cout << "Cleaning gbuffer...";
		graphics_wrapper_->DeleteFramebuffer(gbuffer);
		std::cout << "GBuffer Cleaned.";
	}

	if (graphics_wrapper_) {
		std::cout << "Cleaning Graphics Wrapper...";
		graphics_wrapper_->Cleanup();
		std::cout << "Graphics Wrapper cleaned.";
	}
}