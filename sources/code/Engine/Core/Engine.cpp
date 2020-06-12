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
#include "../Systems/CubemapSystem.hpp"
#include "../Systems/RigidBodySystem.hpp"
#include "../Systems/RenderTerrainSystem.hpp"
#include "../Systems/RenderSpriteSystem.hpp"
#include "../Systems/ScriptSystem.hpp"
#include "../Systems/UISystem.hpp"
// - AssetManagers
#include "../AssetManagers/AudioManager.hpp"
#include "../AssetManagers/MaterialManager.hpp"
#include "../AssetManagers/GraphicsPipelineManager.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include "../AssetManagers/ModelManager.hpp"

#include "../Core/Input.hpp"

#include <GraphicsCommon/GraphicsWrapper.hpp>
#include <WindowModule/BaseWindow.hpp>

// Util Classes
#include "../Rendering/Renderer2D.hpp"

void Engine::initialize(BaseWindow *window) {
	Logger::init("../log/output.log");
	GRIND_PROFILE_BEGIN_SESSION("Loading", "../log/grind-profile-load.json");
	GRIND_LOG("Initializing Grindstone Game Engine...");

	// Seed Random to get proper random numbers
	srand((unsigned int)time(NULL));

	// Load Settings
	settings_ = new Settings();
	GRIND_LOG("Settings loaded.");

	// Load Input Manager
	input_manager_ = new InputManager();
	GRIND_LOG("Input Manager loaded.");

	// Load DLLS
	dll_graphics_ = new DLLGraphics();
	dll_graphics_->setup(settings_->graphics_language_);

	/*window_ = dll_graphics_->createWindow();
	window_->initialize(windowCreateInfo);*/
	window_ = window;
	window_->setInputInterface(input_manager_);

	Grindstone::GraphicsAPI::GraphicsWrapperCreateInfo graphicsWrapperCreateInfo;
	graphicsWrapperCreateInfo.debug = true;
	graphicsWrapperCreateInfo.window = window_;
	graphics_wrapper_ = dll_graphics_->createGraphicsWrapper();
	graphics_wrapper_->initialize(graphicsWrapperCreateInfo);
	std::cout << "Graphics Information: \r\n\t" <<
		graphics_wrapper_->getVendorName() << "\r\n\t" <<
		graphics_wrapper_->getAdapterName() << "\r\n\t" <<
		graphics_wrapper_->getAPIName() << " " << graphics_wrapper_->getAPIVersion() << "\r\n\r\n";
	dll_audio_ = new DLLAudio();
	//audio_wrapper_ = dll_audio_->getWrapper();

	{
		GRIND_PROFILE_SCOPE("Init Basic Graphic Primitives");
		initializeUniformBuffer();
		initializePlaneVertexBuffer();
		deffUBO();
		initializeTBL();
	}

	// Load Asset Managers
	{
		GRIND_PROFILE_SCOPE("Load Asset Managers");
		//audio_manager_ = new AudioManager();
		material_manager_ = new MaterialManager();
		graphics_pipeline_manager_ = new GraphicsPipelineManager();
		texture_manager_ = new TextureManager();
		model_manager_ = new ModelManager(ubb_);
	}

	// Load Systems
	{
		GRIND_PROFILE_SCOPE("Load Systems");
		addSystem(new ControllerSystem());
		addSystem(new ColliderSystem());
		addSystem(new CubemapSystem());
		addSystem(new RigidBodySystem());
		addSystem(new RenderTerrainSystem(ubb_));
		addSystem(new RenderStaticMeshSystem());
		addSystem(new RenderSpriteSystem());
		addSystem(new LightPointSystem());
		addSystem(new LightSpotSystem());
		addSystem(new LightDirectionalSystem());
		addSystem(new TransformSystem());
		addSystem(new CameraSystem());
		addSystem(new ScriptSystem());
		addSystem(new UiSystem());
	}

	start_time_ = std::chrono::high_resolution_clock::now();
	prev_time_ = prev_time_;

	running_ = true;

	GRIND_PROFILE_END_SESSION();

	GRIND_LOG("Engine succesfully setup.");
	GRIND_LOG("==============================");
}


void Engine::initializeUniformBuffer() {
	int s = sizeof(glm::mat4) * 2 + sizeof(glm::vec4) + sizeof(float) + sizeof(glm::vec3);

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "UniformBufferObject";
	ubbci.size = s; //sizeof(glm::mat4);
	ubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Vertex;
	ubb_ = graphics_wrapper_->createUniformBufferBinding(ubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo ubci;
	ubci.isDynamic = true;
	ubci.size = s;
	ubci.binding = ubb_;
	ubo_ = graphics_wrapper_->createUniformBuffer(ubci);
}

void Engine::initializeTBL() {
	subbindings_.reserve(4);
	subbindings_.emplace_back("gbuffer0", 0); // R G B MatID
	subbindings_.emplace_back("gbuffer1", 1); // nX nY nZ MatData
	subbindings_.emplace_back("gbuffer2", 2); // sR sG sB Roughness
	subbindings_.emplace_back("gbuffer3", 3); // Depth

	Grindstone::GraphicsAPI::TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 0;
	tblci.bindings = subbindings_.data();
	tblci.bindingCount = (uint32_t)subbindings_.size();
	tblci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	gbuffer_tbl_ = graphics_wrapper_->createTextureBindingLayout(tblci);
}

void Engine::consoleCommand(std::string command) {
	GRIND_LOG("Console Command: {}", command);
	if (command == "reload") {
		//scenes_[0]->reload();
	}
	else if (command == "reloadGraphics") {
		reloadGraphicsDLL();
	}
}

void Engine::deffUBO() {
	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo deffubbci;
	deffubbci.binding = 0;
	deffubbci.shaderLocation = "DefferedUBO";
	deffubbci.size = sizeof(DefferedUBO);
	deffubbci.stages = Grindstone::GraphicsAPI::ShaderStageBit::Fragment;
	deff_ubb_ = graphics_wrapper_->createUniformBufferBinding(deffubbci);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo deffubci;
	deffubci.isDynamic = true;
	deffubci.size = sizeof(DefferedUBO);
	deffubci.binding = deff_ubb_;
	deff_ubo_handler_ = graphics_wrapper_->createUniformBuffer(deffubci);
}

Grindstone::GraphicsAPI::VertexArrayObject *Engine::getPlaneVAO() {
	return plane_vao_;
}

Grindstone::GraphicsAPI::VertexBufferLayout Engine::getPlaneVertexLayout() {
	return plane_vertex_layout_;
}

void Engine::initializePlaneVertexBuffer() {
	float plane_verts[12] = {
		-1.0, -1.0,
		1.0, -1.0,
		-1.0,  1.0,
		1.0,  1.0,
		-1.0,  1.0,
		1.0, -1.0,
	};
	
	Grindstone::GraphicsAPI::VertexBufferLayout vbd({
		{ Grindstone::GraphicsAPI::VertexFormat::Float2, "vertexPosition", false, Grindstone::GraphicsAPI::AttributeUsage::Position }
	});

	plane_vertex_layout_ = vbd;

	Grindstone::GraphicsAPI::VertexBufferCreateInfo plane_vbo_ci;
	plane_vbo_ci.layout = &plane_vertex_layout_;
	plane_vbo_ci.content = plane_verts;
	plane_vbo_ci.count = 6;
	plane_vbo_ci.size = sizeof(float) * 6 * 2;

	plane_vbo_ = graphics_wrapper_->createVertexBuffer(plane_vbo_ci);

	Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo plane_vao_ci;
	plane_vao_ci.vertex_buffers = &plane_vbo_;
	plane_vao_ci.vertex_buffer_count = 1;
	plane_vao_ci.index_buffer = nullptr;

	if (graphics_wrapper_->shouldUseImmediateMode())
		plane_vao_ = graphics_wrapper_->createVertexArrayObject(plane_vao_ci);
}

Grindstone::GraphicsAPI::UniformBuffer *Engine::getUniformBuffer() {
	return ubo_;
}

Grindstone::GraphicsAPI::UniformBufferBinding *Engine::getUniformBufferBinding() {
	return ubb_;
}

Grindstone::GraphicsAPI::TextureBindingLayout* Engine::getGbufferTBL() {
	return gbuffer_tbl_;
}

Engine &Engine::getInstance() {
	// Create the Engine instance when "getInstance()" is called (ie: when "engine" is used).
	static Engine newEngine;
	return newEngine;
}

Space *Engine::addSpace(const char *path) {
	auto sp = new Space(path);
	spaces_.emplace_back(sp);
	return sp;
}

System *Engine::addSystem(System * system) {
	systems_[system->system_type_] = system;
	return system;
}

System * Engine::getSystem(ComponentHandle type) {
	return systems_[type];
}

std::vector<Space*>& Engine::getSpaces() {
	return spaces_;
}

Space* Engine::getSpace(SceneHandle scene) {
	return spaces_[scene];
}

Space * Engine::getSpace(std::string name) {
	for (auto scene : spaces_)
		if (scene->getName() == name)
			return scene;

	return nullptr;
}

bool Engine::shouldQuit() {
	return !running_;
}

void Engine::run() {
	/*if (profile_frame_) {
		GRIND_PROFILE_BEGIN_SESSION("Running", "../log/grind-profile-run.json");
		profiled_frame = true;
		profile_frame_ = false;
	}*/
	//r2d.updateBuffers();

	// Calculate Timing
	calculateTime();
	double dt = getUpdateTimeDelta();
	window_->setWindowTitle((std::string("Grindstone - ") + std::to_string(int(1 / dt)) + "fps").c_str());

	window_->handleEvents();
	input_manager_->LoopControls(dt);

	for (auto& system : systems_) {
		if (system)
			system->update();
	}

	for (auto space : spaces_) {
		space->getGizmoRenderer().render();
	}

	//r2d.draw();

	getGraphicsWrapper()->swapBuffers();

	/*if (profiled_frame) {
		GRIND_PROFILE_END_SESSION();
		profiled_frame = false;
	}*/
}

void Engine::shutdown() {
	running_ = false;
}

Engine::~Engine() {
	GRIND_LOG("==============================");
	GRIND_LOG("Closing Grindstone...");

	for (auto &space : spaces_) {
		delete space;
	}

	for (auto &system : systems_) {
		delete system;
	}
	
	/*if (dll_audio_) {
		delete dll_audio_;
	}*/

	if (graphics_pipeline_manager_)
		graphics_pipeline_manager_->cleanup();

	if (dll_graphics_) {
		delete dll_graphics_;
	}

	if (settings_) {
		delete settings_;
	}

	GRIND_LOG("Closed Grindstone.");
}

void Engine::calculateTime() {
	GRIND_PROFILE_FUNC();
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

void Engine::shutdownControl(double) {
	shutdown();
}

void Engine::reloadAudioDLL() {
	// Remove all Audio Resources

	// Reload Audio DLL
	// dll_audio_->reload();
	// audio_wrapper_ = dll_audio_->getWrapper();

	// Rebuild Graphics Resources

	// Reload Graphics
}

void Engine::reloadGraphicsDLL() {
	// Remove all Graphics Resources
	getSystem<LightPointSystem>()->destroyGraphics();
	getSystem<LightSpotSystem>()->destroyGraphics();
	getSystem<LightDirectionalSystem>()->destroyGraphics();
	getSystem<CameraSystem>()->destroyGraphics();
	getSystem<CubemapSystem>()->destroyGraphics();

	graphics_wrapper_->deleteUniformBufferBinding(ubb_);
	graphics_wrapper_->deleteUniformBuffer(ubo_);
	graphics_wrapper_->deleteUniformBufferBinding(deff_ubb_);
	graphics_wrapper_->deleteUniformBuffer(deff_ubo_handler_);
	graphics_wrapper_->deleteVertexArrayObject(plane_vao_);
	graphics_wrapper_->deleteVertexBuffer(plane_vbo_);
	graphics_wrapper_->deleteTextureBindingLayout(gbuffer_tbl_);

	graphics_pipeline_manager_->cleanup();
	model_manager_->destroyGraphics();
	
	// Reload Graphics DLL
	dll_graphics_->reload();
	WindowCreateInfo windowCreateInfo;
	windowCreateInfo.fullscreen = WindowFullscreenMode::Borderless;
	//windowCreateInfo.width = settings_->resolution_x_;
	//windowCreateInfo.height = settings_->resolution_y_;
	windowCreateInfo.title = "Grindstone";
	window_ = dll_graphics_->createWindow();
	window_->initialize(windowCreateInfo);

	Grindstone::GraphicsAPI::GraphicsWrapperCreateInfo graphicsWrapperCreateInfo;
	graphicsWrapperCreateInfo.debug = true;
	graphicsWrapperCreateInfo.window = window_;
	graphics_wrapper_ = dll_graphics_->createGraphicsWrapper();
	graphics_wrapper_->initialize(graphicsWrapperCreateInfo);

	// Reload Graphics
	initializeUniformBuffer();
	initializePlaneVertexBuffer();
	deffUBO();
	initializeTBL();

	// Rebuild Graphics Resources
	getSystem<LightPointSystem>()->loadGraphics();
	getSystem<LightSpotSystem>()->loadGraphics();
	getSystem<LightDirectionalSystem>()->loadGraphics();
	getSystem<CameraSystem>()->loadGraphics();
	getSystem<CubemapSystem>()->loadGraphics();

	// Materials System
	model_manager_->reloadGraphics();
}

void Engine::refreshAll(double) {
	reloadGraphicsDLL();
	reloadAudioDLL();
}

void Engine::profileFrame(double) {
	profile_frame_ = true;
}


void* launchEngine() {
	return &engine;
}

void deleteEngine(void* ptr) {
	delete ptr;
}