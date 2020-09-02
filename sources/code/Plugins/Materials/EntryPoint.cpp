#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
using namespace Grindstone;

extern "C" {
    RENDERER_MATERIAL_API void initializeModule(Plugins::Interface* plugin_interface) {
    }

    RENDERER_MATERIAL_API void releaseModule(Plugins::Interface* plugin_interface) {
    }
}