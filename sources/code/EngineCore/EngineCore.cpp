#include "pch.hpp"
#include "EngineCore.hpp"
#include "Logger.hpp"
#include "Profiling.hpp"
#include "BasicComponents.hpp"
#include "ECS/ComponentArray.hpp"
#include <entt/entt.hpp>

using namespace Grindstone;

ECS::IComponentArray* createTransformComponentArray() {
    return new ECS::ComponentArray<TransformComponent>;
}

bool EngineCore::initialize(CreateInfo& create_info) {
    Logger::init("../log/output.log");
    GRIND_PROFILE_BEGIN_SESSION("Loading", "../log/grind-profile-load.json");
    GRIND_LOG("Initializing {0}...", create_info.applicationTitle);

    // Load core (Logging, ECS and Plugin Manager)
    sceneManager = new SceneManager(this);
    pluginManager = new Plugins::Manager(this, ecsCore);

    ecsCore->registerComponentType("Transform", &createTransformComponentArray);

    // Load Game
    if (create_info.applicationModuleName) {
        pluginManager->loadCritical(create_info.applicationModuleName);
    }

    GRIND_LOG("{0} Initialized.", create_info.applicationTitle);
    GRIND_PROFILE_END_SESSION();

    return true;
}

void EngineCore::run() {
    while (!shouldClose) {
        for (auto scene : sceneManager->scenes) {
            scene.second->update();
        }

        for (auto window : windows) {
            window->immediateSwapBuffers();
            window->handleEvents();
        }
    }
}

EngineCore::~EngineCore() {
    GRIND_LOG("Closing...");
    delete pluginManager;
    delete ecsCore;
    GRIND_LOG("Closed.");
}

void EngineCore::registerGraphicsCore(GraphicsAPI::Core*gw) {
    graphicsCore = gw;
}

void EngineCore::registerInputManager(Input::Interface* manager) {
    inputManager = manager;
}

Input::Interface* EngineCore::getInputManager() {
    return inputManager;
}

SceneManager* EngineCore::getSceneManager() {
    return sceneManager;
}

ECS::Core* EngineCore::getEcsCore() {
    return ecsCore;
}

void EngineCore::addWindow(Window* win) {
    windows.push_back(win);
}
