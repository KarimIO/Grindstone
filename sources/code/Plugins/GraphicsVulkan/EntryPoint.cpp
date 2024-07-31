#include "pch.hpp"
#include <EngineCore/PluginSystem/Interface.hpp>
#include "VulkanCore.hpp"
#include "Common/Window/WindowManager.hpp"
#include "Common/Display/DisplayManager.hpp"
#include <EngineCore/Utils/MemoryAllocator.hpp>
using namespace Grindstone::Memory;
using namespace Grindstone;

Plugins::Interface* pluginInterface = nullptr;

extern "C" {
	GRAPHICS_VULKAN_API void InitializeModule(Plugins::Interface* pInterface) {
		Grindstone::Logger::SetLoggerState(pInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pInterface->GetAllocatorState());

		pluginInterface = pInterface;
		pInterface->RegisterGraphicsCore(new GraphicsAPI::VulkanCore());
		pInterface->RegisterWindowManager(new WindowManager());
		pInterface->RegisterDisplayManager(new DisplayManager());
	}

	GRAPHICS_VULKAN_API void ReleaseModule(Plugins::Interface* pluginInterface) {
		delete pluginInterface->GetGraphicsCore();
	}
}
