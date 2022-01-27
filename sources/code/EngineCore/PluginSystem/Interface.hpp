#pragma once

#include "../pch.hpp"
#include "EngineCore/ECS/SystemFactory.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "Common/Window/Window.hpp"
#include "Common/Logging.hpp"
#include <EngineCore/ECS/ComponentFunctions.hpp>

namespace Grindstone {
    class EngineCore;

    namespace GraphicsAPI {
        class Core;
    }

    namespace ECS {
        class SystemRegistrar;
    }

    class WindowManager;
    class DisplayManager;

	namespace Plugins {
        class Manager;

        class ENGINE_CORE_API Interface {
        public:
            Interface(Manager* manager);

            template<typename... Args>
            static void Print(const char* fmt, const Args &... args) {
                Logger::Print(fmt, args...);
            }

            template<typename... Args>
            static void PrintTrace(const char* fmt, const Args &... args) {
                Logger::PrintTrace(fmt, args...);
            }

            template<typename... Args>
            static void PrintWarning(const char* fmt, const Args &... args) {
                Logger::PrintWarning(fmt, args...);
            }

            template<typename... Args>
            static void PrintError(const char* fmt, const Args &... args) {
                Logger::PrintError(fmt, args...);
            }

            template<typename... Args>
            static void Print(LogSeverity logSeverity, const char* fmt, const Args &... args) {
                Logger::Print(logSeverity, fmt, args...);
            }

            virtual EngineCore* getEngineCore();
            virtual GraphicsAPI::Core* getGraphicsCore();
            virtual bool loadPlugin(const char* name);
            virtual void loadPluginCritical(const char* name);
            virtual void registerGraphicsCore(Grindstone::GraphicsAPI::Core* core);
            virtual void registerWindowManager(Grindstone::WindowManager*);
            virtual void registerDisplayManager(Grindstone::DisplayManager*);
            virtual Window* createWindow(Window::CreateInfo&);
            virtual Display getMainDisplay();
            virtual uint8_t countDisplays();
            virtual void enumerateDisplays(Display*displays);
           
            virtual void registerSystem(const char* name, ECS::SystemFactory factory);

			template<typename T>
			void RegisterComponent(ECS::SetupComponentFn setupComponentFn = nullptr) {
				componentRegistrar->RegisterComponent<T>(setupComponentFn);
			}
            ECS::ComponentRegistrar* componentRegistrar = nullptr;
            ECS::SystemRegistrar* systemRegistrar = nullptr;
        private:
            Manager*    manager = nullptr;
            GraphicsAPI::Core* graphicsCore = nullptr;
            Grindstone::Window* (*windowFactoryFn)(Grindstone::Window::CreateInfo&) = nullptr;
            Grindstone::Display(*getMainDisplayFn)() = nullptr;
            uint8_t (*countDisplaysFn)() = nullptr;
            void    (*enumerateDisplaysFn)(Grindstone::Display*) = nullptr;

			friend class Manager;
        };
	}
}
