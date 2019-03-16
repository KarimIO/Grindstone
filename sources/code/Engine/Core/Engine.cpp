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
	LOG("Initializing Grindstone Game Engine...\n");

	// Seed Random to get proper random numbers
	srand((unsigned int)time(NULL));

	// Load Settings
	settings_ = new Settings();
	LOG("Settings loaded.\n");

	input_manager_ = new InputManager();
	LOG("Input Manager loaded.\n");

	// Load DLLS
	dll_graphics_ = new DLLGraphics();
	graphics_wrapper_ = dll_graphics_->getWrapper();
	dll_audio_ = new DLLAudio();
	//audio_wrapper_ = dll_audio_->getWrapper();

	initializeUniformBuffer();
	initializePlaneVertexBuffer();
	deffUBO();
	initializeTBL();

	// Load Managers
	//audio_manager_ = new AudioManager();
	material_manager_ = new MaterialManager();
	graphics_pipeline_manager_ = new GraphicsPipelineManager();
	texture_manager_ = new TextureManager();
	model_manager_ = new ModelManager(ubb_);
	imgui_manager_ = new ImguiManager();
	// - Load Input Manager

	// Load Systems
	addSystem(new ControllerSystem());
	addSystem(new ColliderSystem());
	addSystem(new CubemapSystem());
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

#ifdef INCLUDE_EDITOR
	if (settings_->start_editor_)
		launchEditor();
#endif

	running_ = true;
	LOG("Successfully Loaded.\n");
	LOG("==============================\n");

	graphics_wrapper_->setFocus();
}

#ifdef INCLUDE_EDITOR
void Engine::launchEditor() {
	edit_mode_ = true;
	if (!editor_)
		editor_ = new Editor(imgui_manager_);
}

Editor *Engine::getEditor() {
	return editor_;
}
#endif

ImguiManager *Engine::getImguiManager() {
	return imgui_manager_;
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
	deffubbci.shaderLocation = "UniformBufferObject";
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

		graphics_wrapper_->HandleEvents();
		input_manager_->LoopControls(dt);

		graphics_wrapper_->Clear(CLEAR_BOTH);

		// Add: if (simulating_)
		// Update all Systems
		for (auto scene : scenes_) {
			for (auto &system : systems_) {
				if (system)
					system->update(dt);
			}
		}
		
		if (edit_mode_) {
			editor_->update();
		}

		getGraphicsWrapper()->SwapBuffer();
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
