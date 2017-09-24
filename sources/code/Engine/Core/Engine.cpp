#include "Engine.h"
#include "Utilities.h"
#include "GraphicsDLLPointer.h"
#include "iniHandler.h"
#include <stdio.h>
#include "LevelLoader.h"
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
	Engine *Engine::_instance=0;
#endif

bool Engine::Initialize() {
	srand((unsigned int)time(NULL));

	// Get Settings here:
	InitializeSettings();
	//if (!InitializeAudio())										return false;
	if (!InitializeGraphics(engine.settings.graphicsLanguage))		return false;
	physicsSystem.Initialize();

	lightSystem.SetPointers(graphicsWrapper, &geometryCache);

	renderPathType = RENDERPATH_DEFERRED;
	switch (renderPathType) {
	default:
		//renderPath = (RenderPath *)new RenderPathForward(graphicsWrapper, &geometryCache);
		break;
	case RENDERPATH_DEFERRED:
		renderPath = (RenderPath *)new RenderPathDeferred(graphicsWrapper);
		break;
	};

	postPipeline.Initialize();

	CheckModPaths();

	//sUi.LoadDocument("test.rml");

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

bool Engine::InitializeAudio() {
	LoadDLL(std::string("../audiosdl"));

	AudioSystem *(*pfnCreateAudio)() = (AudioSystem *(*)())LoadDLLFunction("createAudio");
	if (!pfnCreateAudio) {
		fprintf(stderr, "Cannot get createAudio function!\n");
		return false;
	}

	audioSystem = pfnCreateAudio();
	audioSystem->Initialize();
	sounds.push_back(audioSystem->LoadSound("../assets/sounds/snaredrum.wav"));
	sounds.push_back(audioSystem->LoadSound("../assets/sounds/kickdrum.wav"));

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
	graphicsWrapper = (GraphicsWrapper*)pfnCreateGraphics(createInfo);


	graphicsWrapper->CreateDefaultStructures();

	VertexBindingDescription vbd;
	vbd.binding = 0;
	vbd.elementRate = false;
	vbd.stride = sizeof(Vertex);

	std::vector<VertexAttributeDescription> vads(4);
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

	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "UniformBufferObject";
	ubbci.size = sizeof(myUBO);
	ubbci.stages = SHADER_STAGE_VERTEX_BIT;
	UniformBufferBinding *ubb = graphicsWrapper->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = false;
	ubci.size = sizeof(MatUniformBufferObject);
	ubci.binding = ubb;
	ubo = graphicsWrapper->CreateUniformBuffer(ubci);

	UniformBufferBindingCreateInfo ubbci2;
	ubbci2.binding = 1;
	ubbci2.shaderLocation = "ModelMatrixBuffer";
	ubbci2.size = sizeof(modelUBO);
	ubbci2.stages = SHADER_STAGE_VERTEX_BIT;
	UniformBufferBinding *ubb2 = graphicsWrapper->CreateUniformBufferBinding(ubbci2);

	UniformBufferCreateInfo ubci2;
	ubci2.isDynamic = false;
	ubci2.size = sizeof(modelUBO);
	ubci2.binding = ubb2;
	ubo2 = graphicsWrapper->CreateUniformBuffer(ubci2);

	materialManager.Initialize(graphicsWrapper, vbd, vads, ubb);
	geometryCache.Initialize(graphicsWrapper, vbd, vads, &materialManager);

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
	gbuffer = graphicsWrapper->CreateFramebuffer(gbufferCI);

	ColorFormat deviceColorFormat = graphicsWrapper->GetDeviceColorFormat();
	FramebufferCreateInfo defaultFramebufferCI;
	defaultFramebufferCI.colorFormats = &deviceColorFormat;
	defaultFramebufferCI.depthFormat = FORMAT_DEPTH_24;
	defaultFramebufferCI.numColorTargets = 1;
	defaultFramebufferCI.width = engine.settings.resolutionX;
	defaultFramebufferCI.height = engine.settings.resolutionY;
	defaultFramebufferCI.renderPass = nullptr;
	defaultFramebuffer = graphicsWrapper->CreateFramebuffer(defaultFramebufferCI);

	return true;
}

Engine &Engine::GetInstance() {
	static Engine newEngine;
	return newEngine;
}

void Engine::Render(glm::mat4 _projMat, glm::mat4 _viewMat) {
	if (graphicsWrapper->SupportsCommandBuffers()) {
		materialManager.DrawDeferred();
		graphicsWrapper->WaitUntilIdle();
	}
	else {
		if (!engine.settings.debugNoLighting) {
			gbuffer->BindWrite();
			gbuffer->Clear();
			materialManager.DrawImmediate();
			gbuffer->Unbind();
			graphicsWrapper->BindDefaultFramebuffer();
			gbuffer->BindRead();
			renderPath->Draw(gbuffer);
			gbuffer->Unbind();

			graphicsWrapper->SwapBuffer();
		}
		else {
			graphicsWrapper->BindDefaultFramebuffer();
			graphicsWrapper->Clear();
			materialManager.DrawImmediate();
			//graphicsWrapper->Blit(0,0,0,1366,768);
			graphicsWrapper->SwapBuffer();
		}
	}
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
	graphicsWrapper->ResetCursor();

	while (isRunning) {
		CalculateTime();

		graphicsWrapper->HandleEvents();
		inputSystem.LoopControls();
		physicsSystem.StepSimulation(GetUpdateTimeDelta());
		physicsSystem.SetTransforms();

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
	geometryCache.LoadPreloaded();
	//terrainSystem.GenerateComponents();

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
	materialManager.Shutdown();
	geometryCache.Shutdown();
	physicsSystem.Cleanup();

	if (audioSystem)
		audioSystem->Shutdown();

	if (gbuffer)
		graphicsWrapper->DeleteFramebuffer(gbuffer);

	if (defaultFramebuffer)
		graphicsWrapper->DeleteFramebuffer(defaultFramebuffer);

	if (graphicsWrapper)
		graphicsWrapper->Cleanup();
}