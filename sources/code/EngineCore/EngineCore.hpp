#pragma once

#include <vector>

namespace Grindstone {
	namespace GraphicsAPI {
		class Core;
	};

	namespace Input {
		class Interface;
	};

	namespace Plugins {
		class Manager;
	};

    namespace ECS {
        class ComponentRegistrar;
        class SystemRegistrar;
        class Core;
    };

	namespace SceneManagement {
		class SceneManager;
	}
	
	class Window;

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
        virtual SceneManagement::SceneManager* getSceneManager();
        virtual ECS::SystemRegistrar* getSystemRegistrar();
        virtual ECS::ComponentRegistrar* getComponentRegistrar();
        void addWindow(Window* win);
        std::vector<Window *> windows;
    private:
        Plugins::Manager* pluginManager;
        GraphicsAPI::Core* graphicsCore;
        Input::Interface* inputManager;
        SceneManagement::SceneManager* sceneManager;
        ECS::ComponentRegistrar* componentRegistrar;
        ECS::SystemRegistrar* systemRegistrar;
        ECS::Core* ecsCore;
        bool shouldClose = false;
    };
}