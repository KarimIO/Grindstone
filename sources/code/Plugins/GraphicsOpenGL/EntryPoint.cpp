#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include "GLCore.hpp"
#include "Common/Window/WindowManager.hpp"
#include "Common/Display/DisplayManager.hpp"
using namespace Grindstone;

extern "C" {
	GRAPHICS_OPENGL_API void initializeModule(Plugins::Interface* plugin_interface) {
		plugin_interface->registerGraphicsCore(new GraphicsAPI::GLCore());
		plugin_interface->registerWindowManager(new WindowManager());
		plugin_interface->registerDisplayManager(new DisplayManager());
	}

	GRAPHICS_OPENGL_API void releaseModule(Plugins::Interface* plugin_interface) {
		delete plugin_interface->getGraphicsCore();
	}
}
