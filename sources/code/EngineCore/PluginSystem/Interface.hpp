#pragma once

#include "../pch.hpp"
#include <Common/Window/Window.hpp>
#include <Plugins/InputSystem/InputSystem.hpp>
#include "../ECS/ComponentFactory.hpp"
#include "../ECS/SystemFactory.hpp"
#include "../ECS/ComponentRegistrar.hpp"
#include "../ECS/SystemRegistrar.hpp"

namespace Grindstone {
    class EngineCore;

    namespace GraphicsAPI {
        class Core;
    }

	namespace Plugins {
        class Manager;

        class ENGINE_CORE_API Interface {
        public:
            Interface(Manager* manager);

			virtual void addWindow(Window* win);
            virtual EngineCore* getEngineCore();
            virtual GraphicsAPI::Core* getGraphicsCore();
            virtual void log(const char* msg);
            virtual bool loadPlugin(const char* name);
            virtual void loadPluginCritical(const char* name);
            virtual void registerGraphicsCore(Grindstone::GraphicsAPI::Core* core);
            virtual void registerWindowFactory(Grindstone::Window* (*gw)(Grindstone::Window::CreateInfo&));
            virtual void registerDisplayFunctions(Grindstone::Display(*fn_get_main_display)(),
            uint8_t(*fn_count_displays)(),
            void    (*fn_enumerate_displays)(Grindstone::Display*));
            virtual Window* createWindow(Window::CreateInfo&);
            virtual Display getMainDisplay();
            virtual uint8_t countDisplays();
            virtual void enumerateDisplays(Display*displays);
           
            virtual void registerSystem(const char* name, ECS::SystemFactory factory);
            virtual void registerComponentType(const char* name, ECS::ComponentFactory factory);
        private:
            Manager*    manager = nullptr;
            GraphicsAPI::Core* graphicsCore = nullptr;
            ECS::SystemRegistrar* systemRegistrar = nullptr;
            ECS::ComponentRegistrar* componentRegistrar = nullptr;
            Grindstone::Window* (*windowFactoryFn)(Grindstone::Window::CreateInfo&) = nullptr;
            Grindstone::Display(*getMainDisplayFn)() = nullptr;
            uint8_t (*countDisplaysFn)() = nullptr;
            void    (*enumerateDisplaysFn)(Grindstone::Display*) = nullptr;
        };
	}
}