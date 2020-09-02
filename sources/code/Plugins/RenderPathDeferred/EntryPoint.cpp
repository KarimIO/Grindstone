#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
using namespace Grindstone;

extern "C" {
    RENDERPATH_DEFERRED_API void initializeModule(Plugins::Interface* plugin_interface) {
    }

    RENDERPATH_DEFERRED_API void releaseModule(Plugins::Interface* plugin_interface) {
    }
}