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
#include <entt/entt.hpp>

using namespace Grindstone;

bool EngineCore::initialize(CreateInfo& create_info) {
	Logger::init("../log/output.log");
	GRIND_PROFILE_BEGIN_SESSION("Loading", "../log/grind-profile-load.json");
	GRIND_LOG("Initializing {0}...", create_info.applicationTitle);

	// Load core (Logging, ECS and Plugin Manager)
	systemRegistrar = new ECS::SystemRegistrar();
	setupCoreSystems(systemRegistrar);
	componentRegistrar = new ECS::ComponentRegistrar();
	setupCoreComponents(componentRegistrar);
	sceneManager = new SceneManagement::SceneManager(this);

	sceneManager->loadDefaultScene();

	GRIND_LOG("{0} Initialized.", create_info.applicationTitle);
	GRIND_PROFILE_END_SESSION();

	return true;
}

void EngineCore::run() {
	while (!shouldClose) {
		sceneManager->update();

		for (auto window : windows) {
			window->immediateSwapBuffers();
			window->handleEvents();
		}
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

void EngineCore::addWindow(Window* win) {
	windows.push_back(win);
}
