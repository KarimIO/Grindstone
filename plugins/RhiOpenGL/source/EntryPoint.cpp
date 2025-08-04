#include <RhiOpenGL/include/pch.hpp>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <Common/Window/WindowManager.hpp>
#include <Common/Display/DisplayManager.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include <RhiOpenGL/include/GLCore.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;

Plugins::Interface* pluginInterface = nullptr;
WindowManager* windowManager = nullptr;
DisplayManager* displayManager = nullptr;

extern "C" {
	GRAPHICS_OPENGL_API void InitializeModule(Plugins::Interface* pInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pInterface->GetAllocatorState());

		pluginInterface = pInterface;
		pInterface->RegisterGraphicsCore(AllocatorCore::Allocate<GraphicsAPI::OpenGL::Core>());

		windowManager = AllocatorCore::Allocate<WindowManager>();
		displayManager = AllocatorCore::Allocate<DisplayManager>();
		pInterface->RegisterWindowManager(windowManager);
		pInterface->RegisterDisplayManager(displayManager);
	}

	GRAPHICS_OPENGL_API void ReleaseModule(Plugins::Interface* pluginInterface) {
		AllocatorCore::Free(displayManager);
		AllocatorCore::Free(windowManager);
		AllocatorCore::Free(pluginInterface->GetGraphicsCore());
	}
}
