#pragma once

#include "../pch.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/ECS/SystemFactory.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "Common/Window/Window.hpp"
#include "Common/Logging.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include <EngineCore/ECS/ComponentFunctions.hpp>

namespace Grindstone {

	namespace GraphicsAPI {
		class Core;
	}

	namespace ECS {
		class SystemRegistrar;
	}

	class WindowManager;
	class DisplayManager;
	class BaseAssetRenderer;

	namespace Plugins {
		class Manager;

		class ENGINE_CORE_API Interface {
		public:
			Interface(Manager* manager);
			virtual void Print(LogSeverity logSeverity, const char* message);
			virtual EngineCore* GetEngineCore();
			virtual GraphicsAPI::Core* GetGraphicsCore();
			virtual bool LoadPlugin(const char* name);
			virtual void LoadPluginCritical(const char* name);
			virtual void RegisterGraphicsCore(Grindstone::GraphicsAPI::Core* core);
			virtual void RegisterWindowManager(Grindstone::WindowManager*);
			virtual void RegisterDisplayManager(Grindstone::DisplayManager*);
			virtual Window* CreateDisplayWindow(Window::CreateInfo&);
			virtual Display GetMainDisplay();
			virtual uint8_t CountDisplays();
			virtual void EnumerateDisplays(Display* displays);
			virtual void RegisterSystem(const char* name, ECS::SystemFactory factory);
			virtual void RegisterAssetRenderer(BaseAssetRenderer* assetRenderer);
			virtual void RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* assetImporter);

			template<typename T>
			void RegisterComponent(ECS::SetupComponentFn setupComponentFn = nullptr) {
				componentRegistrar->RegisterComponent<T>(setupComponentFn);
			}
			ECS::ComponentRegistrar* componentRegistrar = nullptr;
			ECS::SystemRegistrar* systemRegistrar = nullptr;
		private:
			EngineCore* engineCore = nullptr;
			Manager* manager = nullptr;
			GraphicsAPI::Core* graphicsCore = nullptr;
			Grindstone::Window* (*windowFactoryFn)(Grindstone::Window::CreateInfo&) = nullptr;
			Grindstone::Display(*getMainDisplayFn)() = nullptr;
			uint8_t (*countDisplaysFn)() = nullptr;
			void    (*enumerateDisplaysFn)(Grindstone::Display*) = nullptr;

			friend class Manager;
		};
	}
}
