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
    class DisplayManager;
    class WindowManager;

    class EngineCore {
    public:
        struct CreateInfo {
            bool isEditor = false;
            const char* applicationModuleName = nullptr;
            const char* applicationTitle = nullptr;
        };

        bool initialize(CreateInfo& ci);
        ~EngineCore();
        virtual void run();
		virtual void runLoopIteration();
        virtual void updateWindows();
        void registerGraphicsCore(GraphicsAPI::Core*);
        virtual void registerInputManager(Input::Interface*);
        virtual Input::Interface* getInputManager();
        virtual SceneManagement::SceneManager* getSceneManager();
        virtual ECS::SystemRegistrar* getSystemRegistrar();
        virtual ECS::ComponentRegistrar* getComponentRegistrar();
    public:
        DisplayManager* displayManager;
        WindowManager* windowManager;
    private:
        SceneManagement::SceneManager* sceneManager = nullptr;
        ECS::ComponentRegistrar* componentRegistrar = nullptr;
        ECS::SystemRegistrar* systemRegistrar = nullptr;
        Plugins::Manager* pluginManager = nullptr;
        GraphicsAPI::Core* graphicsCore = nullptr;
        Input::Interface* inputManager = nullptr;
        ECS::Core* ecsCore = nullptr;
        bool shouldClose = false;
    };
}