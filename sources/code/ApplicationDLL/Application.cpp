#include "pch.hpp"

#include <EngineCore/PluginSystem/Interface.hpp>
#include <Common/Graphics/GraphicsWrapper.hpp>
#include <EngineCore/EngineCore.hpp>

using namespace Grindstone;

extern "C" {
    __declspec(dllexport) void initializeModule(Plugins::Interface* plugin_interface) {
        // Load engine plugins
        // plugin_interface->loadPluginCritical("ScriptCSharp");
        plugin_interface->loadPluginCritical("PluginGraphicsOpenGL");
        // plugin_interface->loadPluginCritical("Transform");

        Window::CreateInfo win_ci;
        win_ci.fullscreen = Window::FullscreenMode::Borderless;
        win_ci.title = "Sandbox";
        win_ci.width = 1000;
        win_ci.height = 1000;
        plugin_interface->enumerateDisplays(&win_ci.display);
        auto win = plugin_interface->createWindow(win_ci);

        GraphicsAPI::Core::CreateInfo gw_create_info;
        gw_create_info.debug = true;
        gw_create_info.window = win;

        auto gw = plugin_interface->getGraphicsCore();
        gw->initialize(gw_create_info);

        float clr_color[4] = { 0.f, 0.f, 0.f, 0.f };
        gw->clear(GraphicsAPI::ClearMode::Color, clr_color);

        win->show();
        plugin_interface->addWindow(win);

        // Register Components

        // Register Systems

        // Load custom game plugins
    }

    __declspec(dllexport) void releaseModule(Plugins::Interface* plugin_interface) {
    }
    
}