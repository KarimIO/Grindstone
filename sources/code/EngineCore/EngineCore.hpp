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

	namespace Events {
		class Dispatcher;
	}
	
	class Window;
	class DisplayManager;
	class WindowManager;

	class AssetRendererManager;
	class MaterialManager;
	class TextureManager;
	class ShaderManager;
	class Mesh3dManager;

	class EngineCore {
	public:
		static EngineCore& GetInstance();
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
		virtual Events::Dispatcher* getEventDispatcher();
		virtual ECS::ComponentRegistrar* getComponentRegistrar();
		virtual GraphicsAPI::Core* getGraphicsCore();
	public:
		DisplayManager* displayManager;
		WindowManager* windowManager;
		MaterialManager* materialManager = nullptr;
		TextureManager* textureManager = nullptr;
		ShaderManager* shaderManager = nullptr;
		Mesh3dManager* mesh3dManager = nullptr;
		AssetRendererManager* assetRendererManager = nullptr;
	private:
		SceneManagement::SceneManager* sceneManager = nullptr;
		ECS::ComponentRegistrar* componentRegistrar = nullptr;
		ECS::SystemRegistrar* systemRegistrar = nullptr;
		Events::Dispatcher* eventDispatcher = nullptr;
		Plugins::Manager* pluginManager = nullptr;
		GraphicsAPI::Core* graphicsCore = nullptr;
		Input::Interface* inputManager = nullptr;
		ECS::Core* ecsCore = nullptr;
		bool shouldClose = false;
	};
}