#include <Grindstone.Renderer.Deferred/include/pch.hpp>

#include <Common/Console/Cvars.hpp>
#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/EngineCore.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererRenderPasses.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Bloom.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Blur.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Debug.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/DepthOfField.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Forward.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Gbuffer.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Lighting.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/ScreenSpaceAmbientOcclusion.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/ScreenSpaceReflections.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Shadow.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Skinning.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Smaa.hpp>
#include <Grindstone.Renderer.Deferred/include/Features/Tonemap.hpp>


using namespace Grindstone;
using namespace Grindstone::Memory;
using namespace Grindstone::Renderer;

Grindstone::Renderer::DeferredRendererRenderPasses deferredRenderPasses;

extern "C" {
	RENDERER_DEFERRED_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());
		Grindstone::CvarSystem::SetInstance(pluginInterface->GetCvarSystem());

		Grindstone::EngineCore* engineCore = pluginInterface->GetEngineCore();
		EngineCore::SetInstance(*engineCore);

		deferredRenderPasses = Grindstone::Renderer::InitializeRenderPasses();
		// pluginInterface->RegisterRendererFeature<Debug>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::Bloom>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::Blur>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::DepthOfField>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::Forward>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::Gbuffer>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::Lighting>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::ScreenSpaceAmbientOcclusion>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::ScreenSpaceReflections>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::Shadow>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::Skinning>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::Smaa>();
		pluginInterface->RegisterRendererFeature<Grindstone::Renderer::Tonemap>();
	}

	RENDERER_DEFERRED_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Grindstone::EngineCore* engineCore = pluginInterface->GetEngineCore();
		// pluginInterface->UnregisterRendererFeature<Debug>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::Tonemap>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::Smaa>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::Skinning>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::Shadow>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::ScreenSpaceReflections>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::ScreenSpaceAmbientOcclusion>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::Lighting>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::Gbuffer>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::Forward>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::DepthOfField>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::Blur>();
		pluginInterface->UnregisterRendererFeature<Grindstone::Renderer::Bloom>();
		Grindstone::Renderer::ReleaseRenderPasses(deferredRenderPasses);
	}
}
