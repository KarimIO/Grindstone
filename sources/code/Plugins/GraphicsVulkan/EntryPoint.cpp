#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include "VulkanCore.hpp"
#include "Common/Window/WindowManager.hpp"
#include "Common/Display/DisplayManager.hpp"
using namespace Grindstone;

extern "C" {
	GRAPHICS_VULKAN_API void InitializeModule(Plugins::Interface* pluginInterface) {
		pluginInterface->RegisterGraphicsCore(new GraphicsAPI::VulkanCore());
		pluginInterface->RegisterWindowManager(new WindowManager());
		pluginInterface->RegisterDisplayManager(new DisplayManager());
	}

	GRAPHICS_VULKAN_API void ReleaseModule(Plugins::Interface* pluginInterface) {
		delete pluginInterface->GetGraphicsCore();
	}
}
