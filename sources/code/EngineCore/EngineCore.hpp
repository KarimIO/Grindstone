#pragma once

#include <fmt/format.h>
#include <filesystem>
#include <vector>
#include <functional>
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

	namespace Assets {
		class AssetManager;
	}

	class Window;
	class DisplayManager;
	class WindowManager;

	class AssetRendererManager;
	class BaseRenderer;

	class EngineCore {
	public:
		static EngineCore& GetInstance();
		struct CreateInfo {
			bool isEditor = false;
			const char* applicationModuleName = nullptr;
			const char* applicationTitle = nullptr;
			bool shouldLoadSceneFromDefaults;
			const char* scenePath = nullptr;
			const char* projectPath = nullptr;
		};

		bool Initialize(CreateInfo& ci);
		~EngineCore();
		virtual void Run();
		virtual void RunEditorLoopIteration();
		virtual void RunLoopIteration();
		virtual void UpdateWindows();
		void RegisterGraphicsCore(GraphicsAPI::Core*);
		virtual void RegisterInputManager(Input::Interface*);
		virtual Input::Interface* GetInputManager();
		virtual SceneManagement::SceneManager* GetSceneManager();
		virtual Plugins::Manager* GetPluginManager();
		virtual ECS::SystemRegistrar* GetSystemRegistrar();
		virtual Events::Dispatcher* GetEventDispatcher();
		virtual ECS::ComponentRegistrar* GetComponentRegistrar();
		virtual GraphicsAPI::Core* GetGraphicsCore();
		virtual BaseRenderer* CreateRenderer();
		virtual std::filesystem::path GetProjectPath();
		virtual std::filesystem::path GetBinaryPath();
		virtual std::filesystem::path GetAssetsPath();
		virtual std::filesystem::path GetAssetPath(std::string subPath);
		virtual void ReloadCsharpBinaries();

		template<typename... Args>
		void Print(LogSeverity logSeverity, fmt::format_string<Args...> fmt, Args &&...args) {
			GetInstance().engineCore->Print(logSeverity, textFormat, args);
			va_end(args);
		}
		virtual void Print(LogSeverity logSeverity, const char* str);

		virtual bool OnTryQuit(Grindstone::Events::BaseEvent* ev);
		virtual bool OnForceQuit(Grindstone::Events::BaseEvent* ev);
		virtual void CalculateDeltaTime();
		virtual double GetTimeSinceLaunch();
		virtual double GetDeltaTime();
	public:
		DisplayManager* displayManager = nullptr;
		WindowManager* windowManager = nullptr;
		Assets::AssetManager* assetManager = nullptr;
		AssetRendererManager* assetRendererManager = nullptr;
		std::function<void()> callbackReloadCsharp;
	private:
		double currentTime = 0.0;
		double deltaTime = 0.0;
		std::chrono::steady_clock::time_point firstFrameTime;
		std::chrono::steady_clock::time_point lastFrameTime;
		SceneManagement::SceneManager* sceneManager = nullptr;
		ECS::ComponentRegistrar* componentRegistrar = nullptr;
		ECS::SystemRegistrar* systemRegistrar = nullptr;
		Events::Dispatcher* eventDispatcher = nullptr;
		Plugins::Manager* pluginManager = nullptr;
		GraphicsAPI::Core* graphicsCore = nullptr;
		Input::Interface* inputManager = nullptr;
		bool shouldClose = false;
		std::filesystem::path projectPath;
		std::filesystem::path binaryPath;
		std::filesystem::path assetsPath;
	};
}
