#pragma once

#include <vector>
#include <EngineCore/PluginSystem/Manager.hpp>
#include <EngineCore/ECS/Core.hpp>
#include <EngineCore/Scenes/Manager.hpp>

namespace Grindstone {
    class EngineCore {
    public:
        struct CreateInfo {
            bool is_editor_ = false;
            const char* application_module_name_ = nullptr;
            const char* application_title_ = nullptr;
        };

        bool initialize(CreateInfo& ci);
        ~EngineCore();
        void run();
        void registerGraphicsCore(GraphicsAPI::Core*);
        void addWindow(Window* win);
    private:
        std::vector<Window *> windows_;
        SceneManager* scene_manager_;
        GraphicsAPI::Core* graphics_core_;
        ECS::Core* ecs_core_;
        Plugins::Manager* plugin_manager_;
        bool should_close_ = false;
    };
}