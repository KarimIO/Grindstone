#pragma once

#include "../pch.hpp"

#include <cstdint>
#include <functional>

#include <Common/Display/Display.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Logging.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/Window/Window.hpp>
#include <EngineCore/AssetRenderer/BaseAssetRenderer.hpp>
#include <EngineCore/Assets/AssetImporter.hpp>
#include <EngineCore/ECS/ComponentFunctions.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/ECS/SystemFactory.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/Logger.hpp>

namespace Grindstone {
	namespace Memory::AllocatorCore {
		struct AllocatorState;
	}

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

		class BaseEditorInterface {
		public:
			BaseEditorInterface() = default;
			BaseEditorInterface(const BaseEditorInterface&) = default;
			BaseEditorInterface(BaseEditorInterface&&) = default;
			virtual ~BaseEditorInterface() {}
		};

		class ENGINE_CORE_API Interface {
		public:
			Interface(Manager* manager);

			virtual void SetEditorInterface(BaseEditorInterface* editorInterface);
			virtual BaseEditorInterface* GetEditorInterface() const;

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
			virtual void RegisterEditorSystem(const char* name, ECS::SystemFactory factory);
			virtual void UnregisterSystem(const char* name);
			virtual void UnregisterEditorSystem(const char* name);
			virtual void RegisterAssetRenderer(BaseAssetRenderer* assetRenderer);
			virtual void UnregisterAssetRenderer(BaseAssetRenderer* assetRenderer);
			virtual void RegisterAssetType(AssetType assetType, const char* typeName, AssetImporter* assetImporter);
			virtual void UnregisterAssetType(AssetType assetType);
			virtual void SetReloadCsharpCallback(std::function<void()> callback);
			virtual Grindstone::Logger::LoggerState* GetLoggerState() const;
			virtual Grindstone::Memory::AllocatorCore::AllocatorState* GetAllocatorState() const;

			template<typename ClassType>
			void RegisterComponent(Grindstone::ECS::SetupComponentFn setupComponentFn = nullptr, Grindstone::ECS::DestroyComponentFn destroyComponentFn = nullptr) {
				componentRegistrar->RegisterComponent<ClassType>(setupComponentFn, destroyComponentFn);
			}

			template<typename T>
			void UnregisterComponent() {
				componentRegistrar->UnregisterComponent<T>();
			}

			ECS::ComponentRegistrar* componentRegistrar = nullptr;
			ECS::SystemRegistrar* systemRegistrar = nullptr;
		private:
			EngineCore* engineCore = nullptr;
			BaseEditorInterface* editorInterface = nullptr;
			Manager* manager = nullptr;
			GraphicsAPI::Core* graphicsCore = nullptr;
			Grindstone::Window* (*windowFactoryFn)(Grindstone::Window::CreateInfo&) = nullptr;
			Grindstone::Display(*getMainDisplayFn)() = nullptr;
			uint8_t (*countDisplaysFn)() = nullptr;
			void    (*enumerateDisplaysFn)(Grindstone::Display*) = nullptr;

			friend Manager;
		};
	}
}
