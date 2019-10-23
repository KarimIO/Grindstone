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
// - AssetManagers
#include "../AssetManagers/AudioManager.hpp"
#include "../AssetManagers/MaterialManager.hpp"
#include "../AssetManagers/GraphicsPipelineManager.hpp"
#include "../AssetManagers/TextureManager.hpp"
#include "../AssetManagers/ModelManager.hpp"
#include "../AssetManagers/ImguiManager.hpp"

#include "../Core/Input.hpp"

#include "../GraphicsCommon/GraphicsWrapper.hpp"

#include "Editor.hpp"

// Util Classes
#include "../Utilities/Logger.hpp"

void Engine::initialize() {
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
	graphics_wrapper_ = dll_graphics_->getWrapper();
	dll_audio_ = new DLLAudio();
	//audio_wrapper_ = dll_audio_->getWrapper();

	initializeUniformBuffer();
	initializePlaneVertexBuffer();
	deffUBO();
	initializeTBL();

	// Load Asset Managers
	//audio_manager_ = new AudioManager();
	material_manager_ = new MaterialManager();
	graphics_pipeline_manager_ = new GraphicsPipelineManager();
	texture_manager_ = new TextureManager();
	model_manager_ = new ModelManager(ubb_);
	imgui_manager_ = new ImguiManager();

	// Load Systems
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

	// Load Default Level
	addScene(settings_->default_map_);

	start_time_ = std::chrono::high_resolution_clock::now();
	prev_time_ = prev_time_;

#ifdef INCLUDE_EDITOR
	edit_is_simulating_ = false;

	if (settings_->start_editor_) {
		launchEditor();
	}
#endif

	running_ = true;
	GRIND_LOG("Successfully Loaded.");
	GRIND_LOG("==============================");

	graphics_wrapper_->setFocus();
}

ImguiManager *Engine::getImguiManager() {
	return imgui_manager_;
}

void Engine::initializeUniformBuffer() {
	int s = sizeof(glm::mat4) * 2 + sizeof(glm::vec4) + sizeof(float) + sizeof(glm::vec3);

	UniformBufferBindingCreateInfo ubbci;
	ubbci.binding = 0;
	ubbci.shaderLocation = "UniformBufferObject";
	ubbci.size = s; //sizeof(glm::mat4);
	ubbci.stages = SHADER_STAGE_VERTEX_BIT;
	ubb_ = graphics_wrapper_->CreateUniformBufferBinding(ubbci);

	UniformBufferCreateInfo ubci;
	ubci.isDynamic = true;
	ubci.size = s;
	ubci.binding = ubb_;
	ubo_ = graphics_wrapper_->CreateUniformBuffer(ubci);
}

void Engine::initializeTBL() {
	subbindings_.reserve(4);
	subbindings_.emplace_back("gbuffer0", 0); // R G B MatID
	subbindings_.emplace_back("gbuffer1", 1); // nX nY nZ MatData
	subbindings_.emplace_back("gbuffer2", 2); // sR sG sB Roughness
	subbindings_.emplace_back("gbuffer3", 3); // Depth

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 0;
	tblci.bindings = subbindings_.data();
	tblci.bindingCount = (uint32_t)subbindings_.size();
	tblci.stages = SHADER_STAGE_FRAGMENT_BIT;
	gbuffer_tbl_ = graphics_wrapper_->CreateTextureBindingLayout(tblci);
}

void Engine::deffUBO() {
	UniformBufferBindingCreateInfo deffubbci;
	deffubbci.binding = 0;
	deffubbci.shaderLocation = "DefferedUBO";
	deffubbci.size = sizeof(DefferedUBO);
	deffubbci.stages = SHADER_STAGE_FRAGMENT_BIT;
	deff_ubb_ = graphics_wrapper_->CreateUniformBufferBinding(deffubbci);

	UniformBufferCreateInfo deffubci;
	deffubci.isDynamic = false;
	deffubci.size = sizeof(DefferedUBO);
	deffubci.binding = deff_ubb_;
	deff_ubo_handler_ = graphics_wrapper_->CreateUniformBuffer(deffubci);
}

VertexArrayObject *Engine::getPlaneVAO() {
	return plane_vao_;
}

VertexBindingDescription Engine::getPlaneVBD() {
	return plane_vbd_;
}

VertexAttributeDescription Engine::getPlaneVAD() {
	return plane_vad_;
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

	plane_vbd_.binding = 0;
	plane_vbd_.elementRate = false;
	plane_vbd_.stride = sizeof(float) * 2;

	plane_vad_.binding = 0;
	plane_vad_.location = 0;
	plane_vad_.format = VERTEX_R32_G32;
	plane_vad_.size = 2;
	plane_vad_.name = "vertexPosition";
	plane_vad_.offset = 0;
	plane_vad_.usage = ATTRIB_POSITION;

	VertexBufferCreateInfo plane_vbo_ci;
	plane_vbo_ci.binding = &plane_vbd_;
	plane_vbo_ci.bindingCount = 1;
	plane_vbo_ci.attribute = &plane_vad_;
	plane_vbo_ci.attributeCount = 1;
	plane_vbo_ci.content = plane_verts;
	plane_vbo_ci.count = 6;
	plane_vbo_ci.size = sizeof(float) * 6 * 2;

	VertexArrayObjectCreateInfo plane_vao_ci;
	plane_vao_ci.vertexBuffer = plane_vbo_;
	plane_vao_ci.indexBuffer = nullptr;
	plane_vao_ = graphics_wrapper_->CreateVertexArrayObject(plane_vao_ci);
	plane_vbo_ = graphics_wrapper_->CreateVertexBuffer(plane_vbo_ci);
	plane_vao_ci.vertexBuffer = plane_vbo_;
	plane_vao_ci.indexBuffer = nullptr;
	plane_vao_->BindResources(plane_vao_ci);
	plane_vao_->Unbind();
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

template<class T>
T *Engine::getSystem() {
	return static_cast<T *>(systems_[T::static_system_type_]);
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

Settings *Engine::getSettings() {
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
		graphics_wrapper_->setTitle((std::string("Grindstone - ") + std::to_string(int(1 / dt)) + "fps").c_str());

		graphics_wrapper_->HandleEvents();
		input_manager_->LoopControls(dt);

		// Add: if (simulating_)
		// Update all Systems
		std::vector<Scene *> *scenes = &scenes_;

#ifdef INCLUDE_EDITOR
		if (edit_mode_ && edit_is_simulating_) {
			scenes = &simulate_scenes_;
		}
#endif

		for (auto scene : *scenes) {
			for (auto &system : systems_) {
				if (system)
					system->update(dt);
			}
		}

#ifdef INCLUDE_EDITOR
		if (edit_mode_) { //  && !edit_is_simulating_
			editor_->update();
		}
#endif

		getGraphicsWrapper()->SwapBuffer();
	}

}

void Engine::shutdown() {
	running_ = false;
}

Engine::~Engine() {
	GRIND_LOG("==============================");
	GRIND_LOG("Closing Grindstone...");

	for (auto &scene : scenes_) {
		delete scene;
	}

	for (auto &system : systems_) {
		delete system;
	}
	
	/*if (dll_audio_) {
		delete dll_audio_;
	}*/

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

	graphics_wrapper_->DeleteUniformBufferBinding(ubb_);
	graphics_wrapper_->DeleteUniformBuffer(ubo_);
	graphics_wrapper_->DeleteUniformBufferBinding(deff_ubb_);
	graphics_wrapper_->DeleteUniformBuffer(deff_ubo_handler_);
	graphics_wrapper_->DeleteVertexArrayObject(plane_vao_);
	graphics_wrapper_->DeleteVertexBuffer(plane_vbo_);
	graphics_wrapper_->DeleteTextureBindingLayout(gbuffer_tbl_);

	delete imgui_manager_;

	graphics_pipeline_manager_->cleanup();
	model_manager_->destroyGraphics();
	
	// Reload Graphics DLL
	dll_graphics_->reload();
	graphics_wrapper_ = dll_graphics_->getWrapper();

	// Reload Graphics
	initializeUniformBuffer();
	initializePlaneVertexBuffer();
	deffUBO();
	initializeTBL();

	imgui_manager_ = new ImguiManager();
	if (editor_)
		editor_->reload(imgui_manager_);

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

#ifdef INCLUDE_EDITOR
void Engine::editorControl(double) {
	if (edit_mode_) {
		edit_mode_ = false;
	}
	else {
		edit_mode_ = true;
		launchEditor();
	}
}

void Engine::launchEditor() {
	edit_mode_ = true;
	if (!editor_) {
		editor_ = new Editor(imgui_manager_);
		editor_->setPath(engine.getScenes()[0]->getPath());
	}
}

Editor *Engine::getEditor() {
	return editor_;
}

void Engine::startSimulation() {
	edit_is_simulating_ = true;

	for (auto s : scenes_) {
		Scene *c = new Scene(*s);
		simulate_scenes_.push_back(c);
	}
}

void Engine::stopSimulation() {
	for (auto s : simulate_scenes_) {
		delete s;
	}

	simulate_scenes_.clear();

	edit_is_simulating_ = false;
}

#endif