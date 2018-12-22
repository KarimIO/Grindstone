// STD headers
#include <vector>
#include <string>

// My Class
#include "Engine.hpp"

// Included Classes
#include "Scene.hpp"
#include "Space.hpp"
// - Utils
#include "../Utilities/SettingsFile.hpp"
#include "../Utilities/DLLGraphics.hpp"
#include "../Utilities/DLLAudio.hpp"
// - Systems
#include "../Systems/CameraSystem.hpp"
#include "../Systems/TransformSystem.hpp"
#include "../Systems/RenderStaticMeshSystem.hpp"
#include "../Systems/ControllerSystem.hpp"
#include "../Systems/LightPointSystem.hpp"
#include "../Systems/LightSpotSystem.hpp"
#include "../Systems/LightDirectionalSystem.hpp"
#include "../Systems/ColliderSystem.hpp"
#include "../Systems/RigidBodySystem.hpp"
// - AssetManagers
#include "../AssetManagers/AudioManager.hpp"
#include "../AssetManagers/MaterialManager.hpp"
#include "../AssetManagers/GraphicsPipelineManager.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include "../AssetManagers/ModelManager.hpp"

#include "../Core/Input.hpp"

#include "../GraphicsCommon/GraphicsWrapper.hpp"

// Util Classes
#include "../Utilities/Logger.hpp"

void Engine::initialize() {
	LOG("Initializing Grindstone Game Engine...\n");

	// Seed Random to get proper random numbers
	srand((unsigned int)time(NULL));

	// Load Settings
	settings_ = new Settings();

	input_manager_ = new InputManager();

	// Load DLLS
	dll_graphics_ = new DLLGraphics();
	graphics_wrapper_ = dll_graphics_->getWrapper();
	dll_audio_ = new DLLAudio();
	//audio_wrapper_ = dll_audio_->getWrapper();

	initializeUniformBuffer();

	// Load Managers
	//audio_manager_ = new AudioManager();
	material_manager_ = new MaterialManager();
	graphics_pipeline_manager_ = new GraphicsPipelineManager();
	texture_manager_ = new TextureManager();
	model_manager_ = new ModelManager(ubb_);
	// - Load Input Manager

	// Load Systems
	addSystem(new ControllerSystem());
	addSystem(new ColliderSystem());
	addSystem(new RigidBodySystem());
	addSystem(new RenderStaticMeshSystem());
	addSystem(new LightPointSystem());
	addSystem(new LightSpotSystem());
	addSystem(new LightDirectionalSystem());
	addSystem(new TransformSystem());
	addSystem(new CameraSystem());
	// addSystem(new GeometryStaticSystem());

	// Load Default Level
	addScene(settings_->default_map_);

	start_time_ = std::chrono::high_resolution_clock::now();
	prev_time_ = prev_time_;

	running_ = true;
	LOG("Successfully Loaded.\n");
	LOG("==============================\n");

	graphics_wrapper_->setFocus();
}

void Engine::initializeUniformBuffer() {
	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "UniformBufferObject";
	ubbci.size = 128; //sizeof(glm::mat4);
	ubbci.stages = SHADER_STAGE_VERTEX_BIT;
	ubb_ = graphics_wrapper_->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = true;
	ubci.size = 128;
	ubci.binding = ubb_;
	ubo_ = engine.getGraphicsWrapper()->CreateUniformBuffer(ubci);
}

UniformBuffer *Engine::getUniformBuffer() {
	return ubo_;
}

UniformBufferBinding *Engine::getUniformBufferBinding() {
	return ubb_;
}

Engine &Engine::getInstance() {
	// Create the Engine instance when "getInstance()" is called (ie: when "engine" is used).
	static Engine newEngine;
	return newEngine;
}

Scene *Engine::addScene(std::string path) {
	auto scene = new Scene(path);
	scenes_.push_back(scene);
	return scene;
}

System *Engine::addSystem(System * system) {
	systems_[system->system_type_] = system;
	return system;
}

System * Engine::getSystem(ComponentHandle type) {
	return systems_[type];
}

std::vector<Scene*> &Engine::getScenes() {
	return scenes_;
}

Scene * Engine::getScene(SceneHandle scene) {
	return scenes_[scene];
}

Scene * Engine::getScene(std::string name) {
	for (auto scene : scenes_)
		if (scene->getName() == name)
			return scene;

	return nullptr;
}

const Settings *Engine::getSettings() {
	return settings_;
}

GraphicsWrapper *Engine::getGraphicsWrapper() {
	return graphics_wrapper_;
}

AudioManager *Engine::getAudioManager() {
	return audio_manager_;
}

MaterialManager *Engine::getMaterialManager() {
	return material_manager_;
}

GraphicsPipelineManager *Engine::getGraphicsPipelineManager() {
	return graphics_pipeline_manager_;
}

TextureManager *Engine::getTextureManager() {
	return texture_manager_;
}

ModelManager *Engine::getModelManager() {
	return model_manager_;
}

InputManager * Engine::getInputManager() {
	return input_manager_;
}

void Engine::run() {
	while (running_) {
		// Calculate Timing
		calculateTime();
		double dt = getUpdateTimeDelta();

		graphics_wrapper_->HandleEvents();
		input_manager_->LoopControls(dt);

		// Update all Systems
		for (auto scene : scenes_) {
			for (auto &system : systems_) {
				if (system)
					system->update(dt);
			}
		}
	}

}

void Engine::shutdown() {
	running_ = false;
}

Engine::~Engine() {
	LOG("==============================\n");
	LOG("Closing Grindstone...\n");

	for (auto &scene : scenes_) {
		delete scene;
	}

	for (auto &system : systems_) {
		delete system;
	}
	
	/*if (dll_audio_) {
		delete dll_audio_;
	}*/

	if (dll_graphics_) {
		delete dll_graphics_;
	}

	if (settings_) {
		delete settings_;
	}

	LOG("Closed Grindstone.\n");
}

void Engine::calculateTime() {
	current_time_ = std::chrono::high_resolution_clock::now();
	delta_time_ = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_ - prev_time_);
	prev_time_ = current_time_;
}

double Engine::getTimeCurrent() {
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(current_time_ - start_time_).count() / 1000000000.0;
}

double Engine::getUpdateTimeDelta() {
	return (double)delta_time_.count() / 1000000000.0;
}

void Engine::shutdownControl(double)
{
	shutdown();
}


/*
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
	LOG("The Grindstone Engine is Initializing.\n");

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
	inputSystem.BindAction("Play", NULL, this, &Engine::playSound, KEY_RELEASED);

	inputSystem.AddControl("q", "CaptureCubemaps", NULL, 1);
	inputSystem.BindAction("CaptureCubemaps", NULL, &(cubemapSystem), &CubemapSystem::CaptureCubemaps);

	inputSystem.AddControl("f5", "RefreshContent", NULL, 1);
	inputSystem.BindAction("RefreshContent", NULL, this, &Engine::RefreshContent, KEY_RELEASED);

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

bool Engine::InitializeGraphics() {
	graphics_wrapper_->CreateDefaultStructures();
	graphics_wrapper_->SetCursorShown(false);

	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "UniformBufferObject";
	ubbci.size = 128; //sizeof(glm::mat4);
	ubbci.stages = SHADER_STAGE_VERTEX_BIT;
	UniformBufferBinding *ubb = graphics_wrapper_->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = true;
	ubci.size = sizeof(MainUBO);
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

	bindings.reserve(7);
	bindings.emplace_back("gbuffer0", 0); // R G B MatID
	bindings.emplace_back("gbuffer1", 1); // nX nY nZ MatData
	bindings.emplace_back("gbuffer2", 2); // sR sG sB Roughness
	bindings.emplace_back("gbuffer3", 3); // Depth
	bindings.emplace_back("shadow_map", 4); // Shadow Map
	bindings.emplace_back("shadow_map1", 5); // Shadow Map
	bindings.emplace_back("shadow_map2", 6); // Shadow Map

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

	/*tbci_refl_.clear();
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
			ubo_buffer_.pv = pv;
			ubo_buffer_.eye_pos = eyePos;
			ubo->UpdateUniformBuffer(&ubo_buffer_);
			ubo->Bind();
			ubo2->Bind();
			
			Render();
		}

		//sUi.Update();
		//sUi.Render();
	}
}
*/