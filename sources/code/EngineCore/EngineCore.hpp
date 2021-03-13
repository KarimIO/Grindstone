#pragma once

#include <vector>
#include <EngineCore/PluginSystem/Manager.hpp>
#include <EngineCore/ECS/Core.hpp>
#include <EngineCore/Scenes/Manager.hpp>

namespace Grindstone {
    class EngineCore {
    public:
        struct CreateInfo {
            bool isEditor = false;
            const char* applicationModuleName = nullptr;
            const char* applicationTitle = nullptr;
        };

        bool initialize(CreateInfo& ci);
        ~EngineCore();
        void run();
        void registerGraphicsCore(GraphicsAPI::Core*);
        virtual void registerInputManager(Input::Interface*);
        virtual Input::Interface* getInputManager();
        virtual SceneManager* getSceneManager();
        virtual ECS::Core* getEcsCore();
        void addWindow(Window* win);
        std::vector<Window *> windows;
    private:
        SceneManager *sceneManager;
        GraphicsAPI::Core* graphicsCore;
        Input::Interface* inputManager;
        ECS::Core* ecsCore;
        Plugins::Manager* pluginManager;
        bool shouldClose = false;
    };
}