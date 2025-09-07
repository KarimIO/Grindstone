#include <Grindstone.Renderer.Deferred/include/pch.hpp>

#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/EngineCore.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererFactory.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>

using namespace Grindstone;
using namespace Grindstone::Memory;

extern "C" {
	RENDERER_DEFERRED_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		Grindstone::EngineCore* engineCore = pluginInterface->GetEngineCore();
		EngineCore::SetInstance(*engineCore);

		Grindstone::DeferredRendererFactory* factory = Grindstone::Memory::AllocatorCore::Allocate<Grindstone::DeferredRendererFactory>();
		engineCore->SetRendererFactory(factory);
	}

	RENDERER_DEFERRED_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Grindstone::EngineCore* engineCore = pluginInterface->GetEngineCore();
		Grindstone::BaseRendererFactory* factory = engineCore->GetRendererFactory();

		Grindstone::Memory::AllocatorCore::Free(factory);
	}
}
