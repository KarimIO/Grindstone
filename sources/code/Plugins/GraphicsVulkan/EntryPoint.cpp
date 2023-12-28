#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include "VulkanCore.hpp"
#include "Common/Window/WindowManager.hpp"
#include "Common/Display/DisplayManager.hpp"
using namespace Grindstone;

Plugins::Interface* pluginInterface = nullptr;

static void Print(LogSeverity s, const char* m) {
	pluginInterface->Print(s, m);
}

extern "C" {
	GRAPHICS_VULKAN_API void InitializeModule(Plugins::Interface* pInterface) {
		pluginInterface = pInterface;
		pInterface->RegisterGraphicsCore(new GraphicsAPI::VulkanCore(Print));
		pInterface->RegisterWindowManager(new WindowManager());
		pInterface->RegisterDisplayManager(new DisplayManager());
	}

	GRAPHICS_VULKAN_API void ReleaseModule(Plugins::Interface* pluginInterface) {
		delete pluginInterface->GetGraphicsCore();
	}
}
