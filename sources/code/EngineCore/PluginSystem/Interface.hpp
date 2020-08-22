#pragma once

#include "../pch.hpp"
#include <Common/Window/Window.hpp>

namespace Grindstone {
    class EngineCore;

    namespace ECS {
        class Core;
    }

    namespace GraphicsAPI {
        class Core;
    }

	namespace Plugins {
        class Manager;

        class ENGINE_CORE_API Interface {
        public:
            Interface(Manager* manager, ECS::Core* core);

			virtual void addWindow(Window* win);
            virtual EngineCore* getEngineCore();
            virtual GraphicsAPI::Core* getGraphicsCore();
            virtual ECS::Core* getEcsCore();
            virtual void log(const char* msg);
            virtual bool loadPlugin(const char* name);
            virtual void loadPluginCritical(const char* name);
            virtual void registerGraphicsCore(GraphicsAPI::Core* core);
            virtual void registerWindowFactory(Grindstone::Window* (*gw)(Window::CreateInfo&));
            virtual void registerDisplayFunctions(Grindstone::Display(*fn_get_main_display)(),
            uint8_t(*fn_count_displays)(),
            void    (*fn_enumerate_displays)(Grindstone::Display*));
            virtual Window* createWindow(Window::CreateInfo&);
            virtual Display getMainDisplay();
            virtual uint8_t countDisplays();
            virtual void enumerateDisplays(Display*displays);
           
            template<typename T>
            void registerSystem() {
                core_->registerSystem<T>();
            }

            template<typename T>
            void registerComponentType() {
                core_->registerComponentType<T>();
            }
        private:
            Manager*    manager_ = nullptr;
            GraphicsAPI::Core* graphics_core_ = nullptr;
            ECS::Core*  ecs_core_ = nullptr;
            Grindstone::Window* (*fn_window_factory_)(Grindstone::Window::CreateInfo&) = nullptr;
            Grindstone::Display(*fn_get_main_display_)() = nullptr;
            uint8_t (*fn_count_displays_)() = nullptr;
            void    (*fn_enumerate_displays_)(Grindstone::Display*) = nullptr;
        };
	}
}