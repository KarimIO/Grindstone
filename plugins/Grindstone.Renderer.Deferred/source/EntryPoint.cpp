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
		pluginInterface->RegisterRendererFeature<Bloom>();
		pluginInterface->RegisterRendererFeature<Blur>();
		pluginInterface->RegisterRendererFeature<Debug>();
		pluginInterface->RegisterRendererFeature<DepthOfField>();
		pluginInterface->RegisterRendererFeature<Forward>();
		pluginInterface->RegisterRendererFeature<Gbuffer>();
		pluginInterface->RegisterRendererFeature<Lighting>();
		pluginInterface->RegisterRendererFeature<ScreenSpaceAmbientOcclusion>();
		pluginInterface->RegisterRendererFeature<ScreenSpaceReflections>();
		pluginInterface->RegisterRendererFeature<Shadow>();
		pluginInterface->RegisterRendererFeature<Skinning>();
		pluginInterface->RegisterRendererFeature<Smaa>();
		pluginInterface->RegisterRendererFeature<Tonemap>();
	}

	RENDERER_DEFERRED_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Grindstone::EngineCore* engineCore = pluginInterface->GetEngineCore();
		pluginInterface->UnregisterRendererFeature<Tonemap>();
		pluginInterface->UnregisterRendererFeature<Smaa>();
		pluginInterface->UnregisterRendererFeature<Skinning>();
		pluginInterface->UnregisterRendererFeature<Shadow>();
		pluginInterface->UnregisterRendererFeature<ScreenSpaceReflections>();
		pluginInterface->UnregisterRendererFeature<ScreenSpaceAmbientOcclusion>();
		pluginInterface->UnregisterRendererFeature<Lighting>();
		pluginInterface->UnregisterRendererFeature<Gbuffer>();
		pluginInterface->UnregisterRendererFeature<Forward>();
		pluginInterface->UnregisterRendererFeature<DepthOfField>();
		pluginInterface->UnregisterRendererFeature<Debug>();
		pluginInterface->UnregisterRendererFeature<Blur>();
		pluginInterface->UnregisterRendererFeature<Bloom>();
		Grindstone::Renderer::ReleaseRenderPasses(deferredRenderPasses);
	}
}
