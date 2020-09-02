#include "pch.hpp"
#include "EngineCore.hpp"
#include "Logger.hpp"
#include "Profiling.hpp"
#include "BasicComponents.hpp"
#include "ECS/ComponentArray.hpp"

using namespace Grindstone;

ECS::IComponentArray* createTransformComponentArray() {
    return new ECS::ComponentArray<TransformComponent>;
}

bool EngineCore::initialize(CreateInfo& create_info) {
    Logger::init("../log/output.log");
    GRIND_PROFILE_BEGIN_SESSION("Loading", "../log/grind-profile-load.json");
    GRIND_LOG("Initializing {0}...", create_info.application_title_);

    // Load core (Logging, ECS and Plugin Manager)
    scene_manager_ = new SceneManager(this);
    ecs_core_ = new ECS::Core();
    plugin_manager_ = new Plugins::Manager(this, ecs_core_);

    ecs_core_->registerComponentType("Transform", &createTransformComponentArray);

    // Load Game
    if (create_info.application_module_name_) {
        plugin_manager_->loadCritical(create_info.application_module_name_);
    }

    GRIND_LOG("{0} Initialized.", create_info.application_title_);
    GRIND_PROFILE_END_SESSION();

    return true;
}

void EngineCore::run() {
    while (!should_close_) {
        for (auto s : scene_manager_->scenes_) {
            s.second->update();
        }

        for (auto w : windows_) {
            w->immediateSwapBuffers();
            w->handleEvents();
        }
    }
}

EngineCore::~EngineCore() {
    GRIND_LOG("Closing...");
    delete plugin_manager_;
    delete ecs_core_;
    GRIND_LOG("Closed.");
}

void EngineCore::registerGraphicsCore(GraphicsAPI::Core*gw) {
    graphics_core_ = gw;
}

void EngineCore::registerInputManager(Input::Interface* manager) {
    input_manager_ = manager;
}

Input::Interface* EngineCore::getInputManager() {
    return input_manager_;
}

SceneManager* EngineCore::getSceneManager() {
    return scene_manager_;
}

ECS::Core* EngineCore::getEcsCore() {
    return ecs_core_;
}

void EngineCore::addWindow(Window* win) {
    windows_.push_back(win);
}
