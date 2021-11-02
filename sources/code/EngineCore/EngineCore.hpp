#pragma once

#include <vector>
#include <chrono>
#include "Common/Logging.hpp"

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
		struct BaseEvent;
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
	class Mesh3dRenderer;
	class BaseRenderer;

	class EngineCore {
	public:
		static EngineCore& GetInstance();
		struct CreateInfo {
			bool isEditor = false;
			const char* applicationModuleName = nullptr;
			const char* applicationTitle = nullptr;
		};

		bool Initialize(CreateInfo& ci);
		~EngineCore();
		virtual void Run();
		virtual void RunLoopIteration();
		virtual void UpdateWindows();
		void RegisterGraphicsCore(GraphicsAPI::Core*);
		virtual void RegisterInputManager(Input::Interface*);
		virtual Input::Interface* GetInputManager();
		virtual SceneManagement::SceneManager* GetSceneManager();
		virtual ECS::SystemRegistrar* GetSystemRegistrar();
		virtual Events::Dispatcher* GetEventDispatcher();
		virtual ECS::ComponentRegistrar* GetComponentRegistrar();
		virtual GraphicsAPI::Core* GetGraphicsCore();
		virtual BaseRenderer* CreateRenderer();

		virtual void Print(LogSeverity logSeverity, const char* msg, ...);

		virtual bool OnTryQuit(Grindstone::Events::BaseEvent* ev);
		virtual bool OnForceQuit(Grindstone::Events::BaseEvent* ev);
		virtual void CalculateDeltaTime();
		virtual double GetDeltaTime();
	public:
		DisplayManager* displayManager;
		WindowManager* windowManager;
		MaterialManager* materialManager = nullptr;
		TextureManager* textureManager = nullptr;
		ShaderManager* shaderManager = nullptr;
		Mesh3dManager* mesh3dManager = nullptr;
		Mesh3dRenderer* mesh3dRenderer = nullptr;
		AssetRendererManager* assetRendererManager = nullptr;
	private:
		double deltaTime = 0.0;
		std::chrono::steady_clock::time_point lastFrameTime;
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
