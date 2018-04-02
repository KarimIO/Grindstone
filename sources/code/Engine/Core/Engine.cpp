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
		screen.Render(t);
		screen.Render(t);
	}
}

#define MULTITHEAD_LOAD 0

bool Engine::Initialize() {
	srand((unsigned int)time(NULL));

	// Get Settings here:
	InitializeSettings();
	if (!InitializeGraphics())		return false;
	if (!InitializeAudio())		return false;
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
		renderPath = (RenderPath *)new RenderPathDeferred(graphics_wrapper_, planeVAO);
		break;
	};

	CheckModPaths();

	//sUi.LoadDocument("test.rml");

	inputSystem.AddControl("escape", "Shutdown", NULL, 1);
	inputSystem.BindAction("Shutdown", NULL, this, &Engine::ShutdownControl, KEY_RELEASED);

	inputSystem.AddControl("r", "Play", NULL, 1);
	inputSystem.BindAction("Play", NULL, this, &Engine::PlaySound, KEY_RELEASED);

	inputSystem.AddControl("q", "CaptureCubemaps", NULL, 1);
	inputSystem.BindAction("CaptureCubemaps", NULL, &(cubemapSystem), &CubemapSystem::CaptureCubemaps);

	//terrainSystem.Initialize();
	if (!InitializeScene(defaultMap))	return false;

	cameraSystem.components[0].SetAspectRatio((float)settings.resolutionX/settings.resolutionY);
	
	if (settings.enableReflections)
		cubemapSystem.LoadCubemaps();

	isRunning = true;
	prevTime = std::chrono::high_resolution_clock::now();
	startTime = std::chrono::high_resolution_clock::now();
	printf("Initialization Complete! Starting:\n==================================\n");

	audio_system_.PlayAutoplay();

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
		cfile.GetFloat(  "Input", "mouseSensitivity",	1.0,		settings.mouse_sensitivity);
		settings.fov *= 3.14159f / 360.0f; // Convert to rad, /2 for full fovY.
		std::string graphics;
		cfile.GetString("Renderer", "graphics", "OpenGL", graphics);
		cfile.GetBool("Renderer", "reflections", true, settings.enableReflections);
		cfile.GetBool("Renderer", "shadows", true, settings.enableShadows);
		cfile.GetBool("Renderer", "useSSAO", false, settings.use_ssao);
		cfile.GetBool("Debug", "showMaterialLod", true, settings.showMaterialLoad);
		cfile.GetBool("Debug", "showPipelineLoad", true, settings.showPipelineLoad);
		cfile.GetBool("Debug", "showTextureLoad", false, settings.showTextureLoad);
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
		cfile.SetFloat("Input", "mouseSensitivity",	1.0);
		cfile.SetString("Renderer", "graphics", "OpenGL");
		cfile.SetBool("Renderer", "reflections", true);
		cfile.SetBool("Renderer", "shadows", true);
		cfile.SetBool("Renderer", "useSSAO", false);
		cfile.SetBool("Debug", "showMaterialLod", true);
		cfile.SetBool("Debug", "showPipelineLoad", true);
		cfile.SetBool("Debug", "showTextureLoad", false);
		cfile.SetString("Game", "defaultmap", "../assets/scenes/sponza.json");

		settings.resolutionX = 1366;
		settings.resolutionY = 768;
		settings.graphicsLanguage = GRAPHICS_OPENGL;
		settings.fov = 90.0f * (3.14159f / 360.0f); // Convert to rad, /2 for full fovY.
		settings.enableReflections = true;
		settings.enableShadows = false;
		settings.vsync = true;
		settings.showMaterialLoad=1;
		settings.showPipelineLoad=1;
		settings.showTextureLoad=0;
		settings.use_ssao = true;
		settings.mouse_sensitivity = 1.0f;
		defaultMap = "../assets/scenes/sponza.json";
	}

}

bool Engine::InitializeAudio() {
	std::string library = "openal";
	
	LoadDLL(library);

	AudioWrapper* (*pfnCreateAudio)() = (AudioWrapper* (*)())LoadDLLFunction("createAudio");
	if (!pfnCreateAudio) {
		fprintf(stderr, "Cannot get createAudio function!\n");
		return false;
	}

	pfnDeleteAudio = (void (*)(AudioWrapper*))LoadDLLFunction("deleteAudio");
	if (!pfnDeleteAudio) {
		fprintf(stderr, "Cannot get deleteAudio function!\n");
		return false;
	}

	audio_wrapper_ = (AudioWrapper*)pfnCreateAudio();
	audio_system_.Initialize(audio_wrapper_);
	
	return true;
}

bool Engine::InitializeGraphics() {
	std::string library;
	switch (settings.graphicsLanguage) {
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

	pfnDeleteGraphics = (void (*)(GraphicsWrapper*))LoadDLLFunction("deleteGraphics");
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
	ubbci.size = 128; //sizeof(glm::mat4);
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
	ubbci2.size = 128; // sizeof(glm::mat4);
	ubbci2.stages = SHADER_STAGE_VERTEX_BIT;
	UniformBufferBinding *ubb2 = graphics_wrapper_->CreateUniformBufferBinding(ubbci2);
	
	UniformBufferCreateInfo ubci2;
	ubci2.isDynamic = true;
	ubci2.size = sizeof(glm::mat4);
	ubci2.binding = ubb2;
	ubo2 = graphics_wrapper_->CreateUniformBuffer(ubci2);

	std::vector<UniformBufferBinding *> ubbs = { ubb, ubb2 };

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

	materialManager.Initialize(graphics_wrapper_);
	geometry_system.AddSystem(new SGeometryStatic(&materialManager, graphics_wrapper_, ubbs));
	geometry_system.AddSystem(new SGeometryTerrain(&materialManager, graphics_wrapper_, ubbs));

	std::vector<RenderTargetCreateInfo> gbuffer_images_ci;
	gbuffer_images_ci.reserve(3);
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, settings.resolutionX, settings.resolutionY); // R  G  B matID
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R16G16B16A16, settings.resolutionX, settings.resolutionY); // nX nY nZ
	gbuffer_images_ci.emplace_back(FORMAT_COLOR_R8G8B8A8, settings.resolutionX, settings.resolutionY); // sR sG sB Roughness
	gbuffer_images_ = graphics_wrapper_->CreateRenderTarget(gbuffer_images_ci.data(), gbuffer_images_ci.size());

	DepthTargetCreateInfo depth_image_ci(FORMAT_DEPTH_24_STENCIL_8, settings.resolutionX, settings.resolutionY, false, false);
	depth_image_ = graphics_wrapper_->CreateDepthTarget(depth_image_ci);
	
	FramebufferCreateInfo gbuffer_ci;
	gbuffer_ci.render_target_lists = &gbuffer_images_;
	gbuffer_ci.num_render_target_lists = 1;
	gbuffer_ci.depth_target = depth_image_;
	gbuffer_ci.render_pass = nullptr;
	gbuffer_ = graphics_wrapper_->CreateFramebuffer(gbuffer_ci);


	rt_gbuffer_.framebuffer = gbuffer_;
	rt_gbuffer_.render_targets = &gbuffer_images_;
	rt_gbuffer_.num_render_targets = 1;
	rt_gbuffer_.depth_target = depth_image_;

	bindings.reserve(5);
	bindings.emplace_back("gbuffer0", 0); // R G B MatID
	bindings.emplace_back("gbuffer1", 1); // nX nY nZ MatData
	bindings.emplace_back("gbuffer2", 2); // sR sG sB Roughness
	bindings.emplace_back("gbuffer3", 3); // Depth
	bindings.emplace_back("shadow_map", 4); // Shadow Map

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 1;
	tblci.bindings = bindings.data();
	tblci.bindingCount = (uint32_t)bindings.size();
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	tbl = graphics_wrapper_->CreateTextureBindingLayout(tblci);
	
	UniformBufferBindingCreateInfo deffubbci;
	deffubbci.binding = 0;
	deffubbci.shaderLocation = "UniformBufferObject";
	deffubbci.size = sizeof(DefferedUBO);
	deffubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	deffubb = graphics_wrapper_->CreateUniformBufferBinding(deffubbci);

	UniformBufferCreateInfo deffubci;
	deffubci.isDynamic = false;
	deffubci.size = sizeof(DefferedUBO);
	deffubci.binding = deffubb;
	deffUBO = graphics_wrapper_->CreateUniformBuffer(deffubci);

	float planeVerts[4 * 6] = {
		-1.0, -1.0,
		1.0, -1.0,
		-1.0,  1.0,
		1.0,  1.0,
		-1.0,  1.0,
		1.0, -1.0,
	};

    VertexBufferCreateInfo planeVboCI;
	planeVboCI.binding = &planeVBD;
	planeVboCI.bindingCount = 1;
	planeVboCI.attribute = &planeVAD;
	planeVboCI.attributeCount = 1;
	planeVboCI.content = planeVerts;
	planeVboCI.count = 6;
	planeVboCI.size = sizeof(float) * 6 * 2;

	VertexArrayObjectCreateInfo planeVAOCi;
	planeVAOCi.vertexBuffer = planeVBO;
	planeVAOCi.indexBuffer = nullptr;
	planeVAO = graphics_wrapper_->CreateVertexArrayObject(planeVAOCi);
	planeVBO = graphics_wrapper_->CreateVertexBuffer(planeVboCI);

    planeVAOCi.vertexBuffer = planeVBO;
	planeVAOCi.indexBuffer = nullptr;
	planeVAO->BindResources(planeVAOCi);
	planeVAO->Unbind();
	
	skybox_.geometry_info_.vbds_count = 1;
	skybox_.geometry_info_.vbds = &planeVBD;
	skybox_.geometry_info_.vads_count = 1;
	skybox_.geometry_info_.vads = &planeVAD;
	skybox_.geometry_info_.ubb_count = 1;
	skybox_.geometry_info_.ubbs = &deffubb;
	skybox_.Initialize(&materialManager, graphics_wrapper_, planeVAO, planeVBO);

	//=====================
	// HDR FBO
	//=====================

	std::vector<RenderTargetCreateInfo> hdr_buffer_ci;
	hdr_buffer_ci.reserve(1);
	hdr_buffer_ci.emplace_back(FORMAT_COLOR_R16G16B16, engine.settings.resolutionX, engine.settings.resolutionY);
	hdr_buffer_ = graphics_wrapper_->CreateRenderTarget(hdr_buffer_ci.data(), hdr_buffer_ci.size());

	FramebufferCreateInfo hdr_framebuffer_ci;
	hdr_framebuffer_ci.render_target_lists = &hdr_buffer_;
	hdr_framebuffer_ci.num_render_target_lists = 1;
	hdr_framebuffer_ci.depth_target = depth_image_;
	hdr_framebuffer_ci.render_pass = nullptr;
	hdr_framebuffer_ = graphics_wrapper_->CreateFramebuffer(hdr_framebuffer_ci);

	rt_hdr_.framebuffer = hdr_framebuffer_;
	rt_hdr_.render_targets = &hdr_buffer_;
	rt_hdr_.num_render_targets = 1;
	rt_hdr_.depth_target = nullptr;

	//=====================
	// HDR Texture Binding
	//=====================

	TextureSubBinding *tonemap_sub_binding_ = new TextureSubBinding("lighting", 4);

	tblci.bindingLocation = 4;
	tblci.bindings = tonemap_sub_binding_;
	tblci.bindingCount = 1;
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	tonemap_tbl_ = graphics_wrapper_->CreateTextureBindingLayout(tblci);

	//=====================
	// Bloom Step 1
	//=====================

	/*ShaderStageCreateInfo stages[2];
	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/bloom.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		stages[0].fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		stages[1].fileName = "../assets/shaders/post_processing/bloom.fxc";
	}
	else {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		stages[1].fileName = "../assets/shaders/post_processing/bloom.spv";
	}

	std::vector<char> vfile;
	if (!readFile(stages[0].fileName, vfile)) {
		throw std::runtime_error("Bloom Vertex Shader missing.\n");
		return 0;
	}
	stages[0].content = vfile.data();
	stages[0].size = (uint32_t)vfile.size();
	stages[0].type = SHADER_VERTEX;

	std::vector<char> ffile;
	if (!readFile(stages[1].fileName, ffile)) {
		throw std::runtime_error("Bloom Fragment Shader missing.\n");
		return 0;
	}
	stages[1].content = ffile.data();
	stages[1].size = (uint32_t)ffile.size();
	stages[1].type = SHADER_FRAGMENT;

	GraphicsPipelineCreateInfo bloomGPCI;
	bloomGPCI.cullMode = CULL_BACK;
	bloomGPCI.bindings = &engine.planeVBD;
	bloomGPCI.bindingsCount = 1;
	bloomGPCI.attributes = &engine.planeVAD;
	bloomGPCI.attributesCount = 1;
	bloomGPCI.width = (float)engine.settings.resolutionX;
	bloomGPCI.height = (float)engine.settings.resolutionY;
	bloomGPCI.scissorW = engine.settings.resolutionX;
	bloomGPCI.scissorH = engine.settings.resolutionY;
	bloomGPCI.primitiveType = PRIM_TRIANGLES;
	bloomGPCI.shaderStageCreateInfos = stages;
	bloomGPCI.shaderStageCreateInfoCount = 2;

	bloomGPCI.textureBindings = &tonemap_tbl_;
	bloomGPCI.textureBindingCount = 1;
	bloomGPCI.uniformBufferBindings = nullptr;
	bloomGPCI.uniformBufferBindingCount = 0;
	pipeline_bloom_ = graphics_wrapper_->CreateGraphicsPipeline(bloomGPCI);

	//=====================
	// SSR
	//=====================

	if (engine.settings.graphicsLanguage == GRAPHICS_OPENGL) {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.glsl";
		stages[1].fileName = "../assets/shaders/post_processing/ssr.glsl";
	}
	else if (engine.settings.graphicsLanguage == GRAPHICS_DIRECTX) {
		stages[0].fileName = "../assets/shaders/lights_deferred/pointVert.fxc";
		stages[1].fileName = "../assets/shaders/post_processing/ssr.fxc";
	}
	else {
		stages[0].fileName = "../assets/shaders/lights_deferred/spotVert.spv";
		stages[1].fileName = "../assets/shaders/post_processing/ssr.spv";
	}

	vfile.clear();
	if (!readFile(stages[0].fileName, vfile)) {
		throw std::runtime_error("SSR Vertex Shader missing.\n");
		return 0;
	}
	stages[0].content = vfile.data();
	stages[0].size = (uint32_t)vfile.size();
	stages[0].type = SHADER_VERTEX;

	ffile.clear();
	if (!readFile(stages[1].fileName, ffile)) {
		throw std::runtime_error("SSR Fragment Shader missing.\n");
		return 0;
	}
	stages[1].content = ffile.data();
	stages[1].size = (uint32_t)ffile.size();
	stages[1].type = SHADER_FRAGMENT;

	GraphicsPipelineCreateInfo ssrGPCI;
	ssrGPCI.cullMode = CULL_BACK;
	ssrGPCI.bindings = &engine.planeVBD;
	ssrGPCI.bindingsCount = 1;
	ssrGPCI.attributes = &engine.planeVAD;
	ssrGPCI.attributesCount = 1;
	ssrGPCI.width = (float)engine.settings.resolutionX;
	ssrGPCI.height = (float)engine.settings.resolutionY;
	ssrGPCI.scissorW = engine.settings.resolutionX;
	ssrGPCI.scissorH = engine.settings.resolutionY;
	ssrGPCI.primitiveType = PRIM_TRIANGLES;
	ssrGPCI.shaderStageCreateInfos = stages;
	ssrGPCI.shaderStageCreateInfoCount = 2;

	std::vector<TextureBindingLayout *>tbls = {tbl, tonemap_tbl_};
	ssrGPCI.textureBindings = tbls.data();
	ssrGPCI.textureBindingCount = tbls.size();
	ssrGPCI.uniformBufferBindings = &deffubb;
	ssrGPCI.uniformBufferBindingCount = 1;
	pipeline_ssr_ = graphics_wrapper_->CreateGraphicsPipeline(ssrGPCI);*/

	//=====================
	// Cubemap
	//=====================

	tbci_refl_.clear();
	tbci_refl_.resize(1);
	tbci_refl_.emplace_back("environmentMap", 4);

	TextureBindingLayoutCreateInfo tblci_refl;
	tblci_refl.bindingLocation = 1;
	tblci_refl.bindings = tbci_refl_.data();
	tblci_refl.bindingCount = tbci_refl_.size();
	tblci_refl.stages = SHADER_STAGE_FRAGMENT_BIT;
	reflection_cubemap_layout_ = graphics_wrapper_->CreateTextureBindingLayout(tblci_refl);

	debug_wrapper_.Initialize(gbuffer_);

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
		renderPath->Render(gbuffer_);

		graphics_wrapper_->SwapBuffer();
	}
}

void Engine::Run() {
	graphics_wrapper_->ResetCursor();

	while (isRunning) {
		CalculateTime();

		graphics_wrapper_->HandleEvents();
		inputSystem.LoopControls(GetUpdateTimeDelta());

		gameplay_system.Update(GetUpdateTimeDelta());
		controllerSystem.update(GetUpdateTimeDelta());
		physicsSystem.Update(GetUpdateTimeDelta());
		transformSystem.Update();
		audio_system_.Update();

		// float time = GetTimeCurrent() * 10.0f;
		// sound_source_->SetVolume(glm::sin(time)*0.5f + 0.5f);
		// audio_wrapper_->SetListenerPosition(8.0f * glm::sin(time), 0, -2.0f);

		if (settings.enableShadows)
			lightSystem.DrawShadows();

		glm::mat4 pv;
		if (cameraSystem.components.size() > 0) {
			CCamera *cam = &cameraSystem.components[0];
			materialManager.resetDraws();
			geometry_system.Cull(cam);

			Entity *ent = &entities[cam->entityID];
			glm::vec3 eyePos = transformSystem.components[ent->components_[COMPONENT_TRANSFORM]].position;
			deffUBOBuffer.eyePos.x = eyePos.x;
			deffUBOBuffer.eyePos.y = eyePos.y;
			deffUBOBuffer.eyePos.z = eyePos.z;
			deffUBOBuffer.invProj = glm::inverse(cam->GetProjection());
			deffUBOBuffer.view = glm::inverse(cam->GetView());
			deffUBOBuffer.time = engine.GetTimeCurrent();

			deffUBOBuffer.resolution.x = settings.resolutionX;
			deffUBOBuffer.resolution.y = settings.resolutionY;
			deffUBO->UpdateUniformBuffer(&deffUBOBuffer);

			pv = cam->GetProjection() * cam->GetView();
			ubo->UpdateUniformBuffer(&pv);
			ubo->Bind();
			ubo2->Bind();
			
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

void Engine::PlaySound(double) {
}

Engine::~Engine() {
	materialManager.cleanup();
	geometry_system.cleanup();

	std::cout << "Cleaning Physics System...\n";
	physicsSystem.Cleanup();
	std::cout << "Physics System cleaned.\n";

	if (gbuffer_) {
		std::cout << "Cleaning gbuffer...\n";
		graphics_wrapper_->DeleteFramebuffer(gbuffer_);
		std::cout << "GBuffer Cleaned.\n";
	}

	if (graphics_wrapper_) {
		std::cout << "Cleaning Graphics Wrapper...\n";
		pfnDeleteGraphics(graphics_wrapper_);
		graphics_wrapper_ = NULL;
		std::cout << "Graphics Wrapper cleaned.\n";
	}

	if (audio_wrapper_) {
		std::cout << "Cleaning Audio Wrapper...\n";
		pfnDeleteAudio(audio_wrapper_);
		audio_wrapper_ = NULL;
		std::cout << "Audio Wrapper cleaned.\n";
	}
}