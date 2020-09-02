#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/EngineCore.hpp>
#include "InputSystem.hpp"
using namespace Grindstone;

extern "C" {
    INPUT_SYSTEM_API void initializeModule(Plugins::Interface* plugin_interface) {
        plugin_interface->getEngineCore()->registerInputManager(new Input::Manager());
    }

    INPUT_SYSTEM_API void releaseModule(Plugins::Interface* plugin_interface) {
    }
}