#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include "GLCore.hpp"
#include "Common/Window/WindowManager.hpp"
#include "Common/Display/DisplayManager.hpp"
using namespace Grindstone;

extern "C" {
	GRAPHICS_OPENGL_API void initializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->registerGraphicsCore(new GraphicsAPI::GLCore());
		pluginInterface->registerWindowManager(new WindowManager());
		pluginInterface->registerDisplayManager(new DisplayManager());
	}

	GRAPHICS_OPENGL_API void releaseModule(Plugins::Interface* pluginInterface) {
		delete pluginInterface->getGraphicsCore();
	}
}
