#include "pch.hpp"
#include "EngineCore.hpp"
#include "Logger.hpp"
#include "Profiling.hpp"
#include "ECS/SystemRegistrar.hpp"
#include "ECS/ComponentRegistrar.hpp"
#include "CoreComponents/setupCoreComponents.hpp"
#include "CoreSystems/setupCoreSystems.hpp"
#include "Scenes/Manager.hpp"
#include "PluginSystem/Manager.hpp"
#include "Common/Graphics/Core.hpp"
#include "Common/Display/DisplayManager.hpp"
#include "Common/Window/WindowManager.hpp"
#include <entt/entt.hpp>

using namespace Grindstone;

bool EngineCore::initialize(CreateInfo& create_info) {
	Logger::init("../log/output.log");
	GRIND_PROFILE_BEGIN_SESSION("Loading", "../log/grind-profile-load.json");
	GRIND_LOG("Initializing {0}...", create_info.applicationTitle);

	// Load core (Logging, ECS and Plugin Manager)
	pluginManager = new Plugins::Manager(this);
	pluginManager->load("PluginGraphicsOpenGL");

	systemRegistrar = new ECS::SystemRegistrar();
	setupCoreSystems(systemRegistrar);
	componentRegistrar = new ECS::ComponentRegistrar();
	setupCoreComponents(componentRegistrar);
	sceneManager = new SceneManagement::SceneManager(this);

	sceneManager->loadDefaultScene();

	Window::CreateInfo win_ci;
	win_ci.fullscreen = Window::FullscreenMode::Windowed;
	win_ci.title = "Sandbox";
	win_ci.width = 800;
	win_ci.height = 600;
	win_ci.engine_core = this;
	displayManager->getMainDisplay();
	auto win = windowManager->createWindow(win_ci);

	GraphicsAPI::Core::CreateInfo graphicsCoreInfo{ win, true };
	graphicsCore->initialize(graphicsCoreInfo);
	win->show();

	GRIND_LOG("{0} Initialized.", create_info.applicationTitle);
	GRIND_PROFILE_END_SESSION();

	return true;
}

void EngineCore::run() {
	float clearVal[4] = {0.3, 0.6, 0.9, 1};
	while (!shouldClose) {
		graphicsCore->clear(GraphicsAPI::ClearMode::All, clearVal, 0, 0);
		sceneManager->update();
		windowManager->updateWindows();
	}
}

EngineCore::~EngineCore() {
	GRIND_LOG("Closing...");
	delete componentRegistrar;
	delete systemRegistrar;
	GRIND_LOG("Closed.");
}

void EngineCore::registerGraphicsCore(GraphicsAPI::Core* graphicsCore) {
	this->graphicsCore = graphicsCore;
}

void EngineCore::registerInputManager(Input::Interface* inputManager) {
	this->inputManager = inputManager;
}

Input::Interface* EngineCore::getInputManager() {
	return inputManager;
}

SceneManagement::SceneManager* EngineCore::getSceneManager() {
	return sceneManager;
}

ECS::ComponentRegistrar* EngineCore::getComponentRegistrar() {
	return componentRegistrar;
}

ECS::SystemRegistrar* EngineCore::getSystemRegistrar() {
	return systemRegistrar;
}
