#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include "GLCore.hpp"
using namespace Grindstone;

extern "C" {
    GRAPHICS_OPENGL_API void initializeModule(Plugins::Interface* plugin_interface) {
        plugin_interface->registerGraphicsCore(new GraphicsAPI::GLCore());
        plugin_interface->registerWindowFactory(Window::create);
        plugin_interface->registerDisplayFunctions(Displays::getMainDisplay, Displays::getDisplayCount, Displays::enumerateDisplays);
    }

    GRAPHICS_OPENGL_API void releaseModule(Plugins::Interface* plugin_interface) {
        delete plugin_interface->getGraphicsCore();
    }
}