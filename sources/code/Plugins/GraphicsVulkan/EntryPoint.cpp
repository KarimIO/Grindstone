#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include "VulkanCore.hpp"
using namespace Grindstone;

extern "C" {
	GRAPHICS_VULKAN_API void initializeModule(Plugins::Interface* plugin_interface) {
		plugin_interface->registerGraphicsCore(new GraphicsAPI::VulkanCore());
		plugin_interface->registerWindowFactory(Window::create);
		plugin_interface->registerDisplayFunctions(Displays::getMainDisplay, Displays::getDisplayCount, Displays::enumerateDisplays);
	}

	GRAPHICS_VULKAN_API void releaseModule(Plugins::Interface* plugin_interface) {
		delete plugin_interface->getGraphicsCore();
	}
}