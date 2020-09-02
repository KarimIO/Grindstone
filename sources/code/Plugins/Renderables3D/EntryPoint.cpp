#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
using namespace Grindstone;

extern "C" {
    RENDERABLES_3D_API void initializeModule(Plugins::Interface* plugin_interface) {
    }

    RENDERABLES_3D_API void releaseModule(Plugins::Interface* plugin_interface) {
    }
}