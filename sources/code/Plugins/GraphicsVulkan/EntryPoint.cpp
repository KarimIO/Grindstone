#include "pch.hpp"

#include <Common/Window/WindowManager.hpp>
#include <Common/Display/DisplayManager.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "VulkanCore.hpp"

using namespace Grindstone::Memory;
using namespace Grindstone;

Plugins::Interface* pluginInterface = nullptr;
WindowManager* windowManager = nullptr;
DisplayManager* displayManager = nullptr;

extern "C" {
	GRAPHICS_VULKAN_API void InitializeModule(Plugins::Interface* pInterface) {
		Grindstone::HashedString::SetHashMap(pInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pInterface->GetAllocatorState());

		pluginInterface = pInterface;
		pInterface->RegisterGraphicsCore(AllocatorCore::Allocate<GraphicsAPI::Vulkan::Core>());

		windowManager = AllocatorCore::Allocate<WindowManager>();
		displayManager = AllocatorCore::Allocate<DisplayManager>();
		pInterface->RegisterWindowManager(windowManager);
		pInterface->RegisterDisplayManager(displayManager);
	}

	GRAPHICS_VULKAN_API void ReleaseModule(Plugins::Interface* pluginInterface) {
		AllocatorCore::Free(displayManager);
		AllocatorCore::Free(windowManager);
		AllocatorCore::Free(pluginInterface->GetGraphicsCore());

	}
}
