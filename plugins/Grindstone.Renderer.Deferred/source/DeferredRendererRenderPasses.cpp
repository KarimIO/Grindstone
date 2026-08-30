#include <array>

#include <Common/Graphics/Formats.hpp>
#include <Common/Graphics/Core.hpp>
#include <Common/Graphics/RenderPass.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>
#include <EngineCore/EngineCore.hpp>

#include <Grindstone.Renderer.Deferred/include/DeferredRendererCommon.hpp>
#include <Grindstone.Renderer.Deferred/include/DeferredRendererRenderPasses.hpp>

using namespace Grindstone;

static Grindstone::GraphicsAPI::RenderPass* CreateDofSeparationRenderPass(Grindstone::GraphicsAPI::Core* graphicsCore, Grindstone::RenderPassRegistry* rpRegistry) {
	std::array<GraphicsAPI::RenderPass::AttachmentInfo, 2> dofSepFormats{};
	dofSepFormats[0] = { GraphicsAPI::Format::R16G16B16A16_SFLOAT, true };
	dofSepFormats[1] = { GraphicsAPI::Format::R16G16B16A16_SFLOAT, true };

	GraphicsAPI::RenderPass::CreateInfo dofSepRenderPassCreateInfo{};
	dofSepRenderPassCreateInfo.debugName = "Depth of Field Separation Render Pass";
	dofSepRenderPassCreateInfo.colorAttachments = dofSepFormats.data();
	dofSepRenderPassCreateInfo.colorAttachmentCount = static_cast<uint32_t>(dofSepFormats.size());
	dofSepRenderPassCreateInfo.depthFormat = GraphicsAPI::Format::Invalid;
	Grindstone::GraphicsAPI::RenderPass* rp = graphicsCore->CreateRenderPass(dofSepRenderPassCreateInfo);
	rpRegistry->RegisterRenderpass(dofSeparationRenderPassKey, rp);
	return rp;
}

static Grindstone::GraphicsAPI::RenderPass* CreateDofBlurAndCombinationRenderPass(Grindstone::GraphicsAPI::Core* graphicsCore, Grindstone::RenderPassRegistry* rpRegistry) {
	GraphicsAPI::RenderPass::AttachmentInfo dofBlurAndComboFormat = { GraphicsAPI::Format::R16G16B16A16_SFLOAT, true };

	GraphicsAPI::RenderPass::CreateInfo dofBlurAndComboRenderPassCreateInfo{};
	dofBlurAndComboRenderPassCreateInfo.debugName = "Depth of Field Blur and Combo Render Pass";
	dofBlurAndComboRenderPassCreateInfo.colorAttachments = &dofBlurAndComboFormat;
	dofBlurAndComboRenderPassCreateInfo.colorAttachmentCount = 1;
	dofBlurAndComboRenderPassCreateInfo.depthFormat = GraphicsAPI::Format::Invalid;
	Grindstone::GraphicsAPI::RenderPass* rp = graphicsCore->CreateRenderPass(dofBlurAndComboRenderPassCreateInfo);
	rpRegistry->RegisterRenderpass(dofBlurAndCombinationRenderPassKey, rp);
	return rp;
}

static Grindstone::GraphicsAPI::RenderPass* CreateSsaoRenderPass(Grindstone::GraphicsAPI::Core* graphicsCore, Grindstone::RenderPassRegistry* rpRegistry) {
	GraphicsAPI::RenderPass::AttachmentInfo attachment{ ambientOcclusionFormat, true };
	GraphicsAPI::RenderPass::CreateInfo ssaoRenderPassCreateInfo{};
	ssaoRenderPassCreateInfo.debugName = "SSAO Renderpass";
	ssaoRenderPassCreateInfo.colorAttachments = &attachment;
	ssaoRenderPassCreateInfo.colorAttachmentCount = 1;
	ssaoRenderPassCreateInfo.depthFormat = GraphicsAPI::Format::Invalid;
	ssaoRenderPassCreateInfo.shouldClearDepthOnLoad = false;
	Grindstone::GraphicsAPI::RenderPass* rp = graphicsCore->CreateRenderPass(ssaoRenderPassCreateInfo);
	rpRegistry->RegisterRenderpass(ssaoRenderPassKey, rp);
	return rp;
}

static Grindstone::GraphicsAPI::RenderPass* CreateSsaoBlurRenderPass(Grindstone::GraphicsAPI::Core* graphicsCore, Grindstone::RenderPassRegistry* rpRegistry) {
	GraphicsAPI::RenderPass::AttachmentInfo attachment{ ambientOcclusionFormat, true };
	GraphicsAPI::RenderPass::CreateInfo ssaoRenderPassCreateInfo{};
	ssaoRenderPassCreateInfo.debugName = "SSAO Blur Renderpass";
	ssaoRenderPassCreateInfo.colorAttachments = &attachment;
	ssaoRenderPassCreateInfo.colorAttachmentCount = 1;
	ssaoRenderPassCreateInfo.depthFormat = GraphicsAPI::Format::Invalid;
	ssaoRenderPassCreateInfo.shouldClearDepthOnLoad = false;
	Grindstone::GraphicsAPI::RenderPass* rp = graphicsCore->CreateRenderPass(ssaoRenderPassCreateInfo);
	rpRegistry->RegisterRenderpass(ssaoBlurRenderPassKey, rp);
	return rp;
}

static Grindstone::GraphicsAPI::RenderPass* CreateGbufferRenderPass(Grindstone::GraphicsAPI::Core* graphicsCore, Grindstone::RenderPassRegistry* rpRegistry) {
	const int gbufferColorCount = 3;
	std::array<GraphicsAPI::RenderPass::AttachmentInfo, gbufferColorCount> gbufferColorAttachments{};
	gbufferColorAttachments[0] = { GraphicsAPI::Format::R8G8B8A8_UNORM, true }; // Albedo
	gbufferColorAttachments[1] = { GraphicsAPI::Format::R16G16B16A16_SNORM, true }; // Normal
	gbufferColorAttachments[2] = { GraphicsAPI::Format::R8G8B8A8_UNORM, true }; // Specular RGB + Roughness Alpha

	GraphicsAPI::RenderPass::CreateInfo gbufferRenderPassCreateInfo{};
	gbufferRenderPassCreateInfo.debugName = "GBuffer Render Pass";
	gbufferRenderPassCreateInfo.colorAttachments = gbufferColorAttachments.data();
	gbufferRenderPassCreateInfo.colorAttachmentCount = static_cast<uint32_t>(gbufferColorAttachments.size());
	gbufferRenderPassCreateInfo.depthFormat = depthFormat;
	gbufferRenderPassCreateInfo.shouldClearDepthOnLoad = true;
	Grindstone::GraphicsAPI::RenderPass* rp = graphicsCore->CreateRenderPass(gbufferRenderPassCreateInfo);
	rpRegistry->RegisterRenderpass(gbufferRenderPassKey, rp);
	rpRegistry->RegisterRenderpass(geometryOpaqueRenderPassKey, rp);
	rpRegistry->RegisterRenderpass(geometryTransparentRenderPassKey, rp);
	rpRegistry->RegisterRenderpass(geometryUnlitRenderPassKey, rp);
	return rp;
}

static Grindstone::GraphicsAPI::RenderPass* CreateMainRenderPass(Grindstone::GraphicsAPI::Core* graphicsCore, Grindstone::RenderPassRegistry* rpRegistry) {
	static float debugColor[4] = { 0.3f, 0.6f, 0.9f, 1.0f };
	GraphicsAPI::RenderPass::AttachmentInfo attachment{ GraphicsAPI::Format::R8G8B8A8_UNORM , true };

	GraphicsAPI::RenderPass::CreateInfo mainRenderPassCreateInfo{};
	mainRenderPassCreateInfo.debugName = "Main HDR Render Pass";
	mainRenderPassCreateInfo.colorAttachments = &attachment;
	mainRenderPassCreateInfo.colorAttachmentCount = 1;
	mainRenderPassCreateInfo.depthFormat = Grindstone::GraphicsAPI::Format::Invalid;
	mainRenderPassCreateInfo.shouldClearDepthOnLoad = false;
	memcpy(mainRenderPassCreateInfo.debugColor, debugColor, sizeof(float) * 4);
	Grindstone::GraphicsAPI::RenderPass* rp = graphicsCore->CreateRenderPass(mainRenderPassCreateInfo);
	rpRegistry->RegisterRenderpass(mainRenderPassKey, rp);
	return rp;
}

static Grindstone::GraphicsAPI::RenderPass* CreateLightingRenderPass(Grindstone::GraphicsAPI::Core* graphicsCore, Grindstone::RenderPassRegistry* rpRegistry) {
	static float debugColor[4] = { 1.0f, 0.9f, 0.5f, 1.0f };
	GraphicsAPI::RenderPass::AttachmentInfo attachment{ litHdrFormat , true };

	GraphicsAPI::RenderPass::CreateInfo lightingRenderPassCreateInfo{};
	lightingRenderPassCreateInfo.debugName = "Deferred Light Render Pass";
	lightingRenderPassCreateInfo.colorAttachments = &attachment;
	lightingRenderPassCreateInfo.colorAttachmentCount = 1;
	lightingRenderPassCreateInfo.depthFormat = GraphicsAPI::Format::D32_SFLOAT;
	lightingRenderPassCreateInfo.shouldClearDepthOnLoad = false;
	memcpy(lightingRenderPassCreateInfo.debugColor, debugColor, sizeof(float) * 4);
	Grindstone::GraphicsAPI::RenderPass* rp = graphicsCore->CreateRenderPass(lightingRenderPassCreateInfo);
	rpRegistry->RegisterRenderpass(lightingRenderPassKey, rp);
	return rp;
}

static Grindstone::GraphicsAPI::RenderPass* CreateForwardLitRenderPass(Grindstone::GraphicsAPI::Core* graphicsCore, Grindstone::RenderPassRegistry* rpRegistry) {
	static float debugColor[4] = { 1.0f, 0.5f, 0.9f, 1.0f };
	GraphicsAPI::RenderPass::AttachmentInfo attachment{ litHdrFormat , true };

	GraphicsAPI::RenderPass::CreateInfo forwardLitRenderPassCreateInfo{};
	forwardLitRenderPassCreateInfo.debugName = "Forward Lit Renderables Render Pass";
	forwardLitRenderPassCreateInfo.colorAttachments = &attachment;
	forwardLitRenderPassCreateInfo.colorAttachmentCount = 1;
	forwardLitRenderPassCreateInfo.depthFormat = depthFormat;
	forwardLitRenderPassCreateInfo.shouldClearDepthOnLoad = false;
	memcpy(forwardLitRenderPassCreateInfo.debugColor, debugColor, sizeof(float) * 4);
	Grindstone::GraphicsAPI::RenderPass* rp = graphicsCore->CreateRenderPass(forwardLitRenderPassCreateInfo);
	rpRegistry->RegisterRenderpass(forwardLitRenderPassKey, rp);
	rpRegistry->RegisterRenderpass(geometrySkyRenderPassKey, rp);
	return rp;
}

static Grindstone::GraphicsAPI::RenderPass* CreateShadowMapRenderPass(Grindstone::GraphicsAPI::Core* graphicsCore, Grindstone::RenderPassRegistry* rpRegistry) {
	GraphicsAPI::RenderPass::CreateInfo renderPassCreateInfo{};
	renderPassCreateInfo.debugName = "Shadow Map Render Pass";
	renderPassCreateInfo.colorAttachments = nullptr;
	renderPassCreateInfo.colorAttachmentCount = 0;
	renderPassCreateInfo.depthFormat = GraphicsAPI::Format::D32_SFLOAT;
	Grindstone::GraphicsAPI::RenderPass* rp = graphicsCore->CreateRenderPass(renderPassCreateInfo);
	rpRegistry->RegisterRenderpass(shadowMapRenderPassKey, rp);
	return rp;
}

Grindstone::Renderer::DeferredRendererRenderPasses Grindstone::Renderer::InitializeRenderPasses() {
	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::RenderPassRegistry* rpRegistry = engineCore.GetRenderPassRegistry();
	Grindstone::Renderer::DeferredRendererRenderPasses rps;

	rps.dofBlurAndCombinationRenderPass = CreateDofBlurAndCombinationRenderPass(graphicsCore, rpRegistry);
	rps.dofSeparationRenderPass = CreateDofSeparationRenderPass(graphicsCore, rpRegistry);
	rps.forwardLitRenderPass = CreateForwardLitRenderPass(graphicsCore, rpRegistry);
	rps.lightingRenderPass = CreateLightingRenderPass(graphicsCore, rpRegistry);
	rps.mainRenderpass = CreateMainRenderPass(graphicsCore, rpRegistry);
	rps.shadowMapRenderPass = CreateShadowMapRenderPass(graphicsCore, rpRegistry);
	rps.ssaoRenderPass = CreateSsaoRenderPass(graphicsCore, rpRegistry);
	rps.ssaoBlurRenderPass = CreateSsaoBlurRenderPass(graphicsCore, rpRegistry);
	rps.gbufferRenderpass = CreateGbufferRenderPass(graphicsCore, rpRegistry);

	return rps;
}

void Grindstone::Renderer::ReleaseRenderPasses(DeferredRendererRenderPasses& rps) {
	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Grindstone::RenderPassRegistry* rpRegistry = engineCore.GetRenderPassRegistry();

	rpRegistry->UnregisterRenderpass(gbufferRenderPassKey);
	rpRegistry->UnregisterRenderpass(geometryOpaqueRenderPassKey);
	rpRegistry->UnregisterRenderpass(geometryUnlitRenderPassKey);
	rpRegistry->UnregisterRenderpass(geometrySkyRenderPassKey);
	rpRegistry->UnregisterRenderpass(geometryTransparentRenderPassKey);
	rpRegistry->UnregisterRenderpass(mainRenderPassKey);

	rpRegistry->UnregisterRenderpass(dofSeparationRenderPassKey);
	rpRegistry->UnregisterRenderpass(dofBlurAndCombinationRenderPassKey);

	rpRegistry->UnregisterRenderpass(lightingRenderPassKey);
	rpRegistry->UnregisterRenderpass(forwardLitRenderPassKey);
	rpRegistry->UnregisterRenderpass(ssaoRenderPassKey);
	rpRegistry->UnregisterRenderpass(shadowMapRenderPassKey);

	graphicsCore->DeleteRenderPass(rps.dofSeparationRenderPass);
	graphicsCore->DeleteRenderPass(rps.dofBlurAndCombinationRenderPass);
	graphicsCore->DeleteRenderPass(rps.shadowMapRenderPass);
	graphicsCore->DeleteRenderPass(rps.lightingRenderPass);
	graphicsCore->DeleteRenderPass(rps.forwardLitRenderPass);
	graphicsCore->DeleteRenderPass(rps.ssaoRenderPass);
	graphicsCore->DeleteRenderPass(rps.mainRenderpass);
	graphicsCore->DeleteRenderPass(rps.gbufferRenderpass);

	rps.dofSeparationRenderPass = nullptr;
	rps.dofBlurAndCombinationRenderPass = nullptr;
	rps.shadowMapRenderPass = nullptr;
	rps.lightingRenderPass = nullptr;
	rps.forwardLitRenderPass = nullptr;
	rps.ssaoRenderPass = nullptr;
	rps.mainRenderpass = nullptr;
	rps.gbufferRenderpass = nullptr;
}
