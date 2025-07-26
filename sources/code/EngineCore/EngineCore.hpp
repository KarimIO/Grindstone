#pragma once

#include <filesystem>
#include <functional>
#include <chrono>

#include <entt/entity/registry.hpp>

#include <Common/Logging.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Core;
		class RenderPass;
	}

	namespace Input {
		class Interface;
	}

	namespace Plugins {
		class Manager;
		class BaseEditorInterface;
	}

	namespace ECS {
		class ComponentRegistrar;
		class SystemRegistrar;
	}

	namespace SceneManagement {
		class SceneManager;
	}

	namespace Events {
		struct BaseEvent;
		class Dispatcher;
	}

	namespace Assets {
		class AssetManager;
		class AssetLoader;
	}

	namespace Profiler {
		class Manager;
	}

	class Window;
	class DisplayManager;
	class WindowManager;

	class AssetRendererManager;
	class BaseRendererFactory;
	class RenderPassRegistry;
	class WorldContextManager;

	class EngineCore {
	public:
		static EngineCore& GetInstance();
		static void SetInstance(EngineCore& engineCore);

		struct CreateInfo {
			bool isEditor = false;
			const char* applicationModuleName = nullptr;
			const char* applicationTitle = nullptr;
			const char* projectPath = nullptr;
			const char* engineBinaryPath = nullptr;
			Assets::AssetLoader* assetLoader = nullptr;
			Grindstone::Plugins::BaseEditorInterface* editorPluginInterface = nullptr;
		};

		bool Initialize(CreateInfo& ci);
		virtual void LoadPluginList();
		virtual void InitializeScene(bool shouldLoadSceneFromDefaults, const char* scenePath = nullptr);
		virtual void ShowMainWindow();

		virtual ~EngineCore();
		virtual void Run();
		virtual void RunEditorLoopIteration();
		virtual void RunLoopIteration();
		virtual void UpdateWindows();
		void RegisterGraphicsCore(GraphicsAPI::Core*);
		virtual void RegisterInputManager(Input::Interface*);
		virtual void SetRendererFactory(BaseRendererFactory* factory);
		virtual Input::Interface* GetInputManager() const;
		virtual SceneManagement::SceneManager* GetSceneManager() const;
		virtual Plugins::Manager* GetPluginManager() const;
		virtual ECS::SystemRegistrar* GetSystemRegistrar() const;
		virtual Events::Dispatcher* GetEventDispatcher() const;
		virtual ECS::ComponentRegistrar* GetComponentRegistrar() const;
		virtual GraphicsAPI::Core* GetGraphicsCore() const;
		virtual Profiler::Manager* GetProfiler() const;
		virtual BaseRendererFactory* GetRendererFactory() const;
		virtual RenderPassRegistry* GetRenderPassRegistry() const;
		virtual WorldContextManager* GetWorldContextManager() const;
		virtual std::filesystem::path GetProjectPath() const;
		virtual std::filesystem::path GetBinaryPath() const;
		virtual std::filesystem::path GetEngineBinaryPath() const;
		virtual std::filesystem::path GetAssetsPath() const;
		virtual std::filesystem::path GetEngineAssetsPath() const;
		virtual std::filesystem::path GetAssetPath(std::string subPath) const;
		virtual entt::registry& GetEntityRegistry();
		virtual void ReloadCsharpBinaries();

		virtual bool OnTryQuit(Grindstone::Events::BaseEvent* ev);
		virtual bool OnForceQuit(Grindstone::Events::BaseEvent* ev);
		virtual void CalculateDeltaTime();
		virtual double GetTimeSinceLaunch() const;
		virtual double GetDeltaTime() const;
	public:
		DisplayManager* displayManager = nullptr;
		WindowManager* windowManager = nullptr;
		Assets::AssetManager* assetManager = nullptr;
		AssetRendererManager* assetRendererManager = nullptr;
		WorldContextManager* worldContextManager = nullptr;
		Profiler::Manager* profiler = nullptr;
		std::function<void()> callbackReloadCsharp;
		bool isEditor = false;
	private:
		double currentTime = 0.0;
		double deltaTime = 0.0;
		std::chrono::steady_clock::time_point firstFrameTime;
		std::chrono::steady_clock::time_point lastFrameTime;
		SceneManagement::SceneManager* sceneManager = nullptr;
		ECS::ComponentRegistrar* componentRegistrar = nullptr;
		ECS::SystemRegistrar* systemRegistrar = nullptr;
		BaseRendererFactory* rendererFactory = nullptr;
		RenderPassRegistry* renderpassRegistry = nullptr;
		Events::Dispatcher* eventDispatcher = nullptr;
		Plugins::Manager* pluginManager = nullptr;
		GraphicsAPI::Core* graphicsCore = nullptr;
		Input::Interface* inputManager = nullptr;
		bool shouldClose = false;
		std::filesystem::path projectPath;
		std::filesystem::path binaryPath;
		std::filesystem::path engineBinaryPath;
		std::filesystem::path engineAssetsPath;
		std::filesystem::path assetsPath;
	};
}
