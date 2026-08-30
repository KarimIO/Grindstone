#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Grindstone::GraphicsAPI {
	class RenderPass;
}

namespace Grindstone::Renderer {
	struct DeferredRendererRenderPasses {
		GraphicsAPI::RenderPass* dofSeparationRenderPass = nullptr;
		GraphicsAPI::RenderPass* dofBlurAndCombinationRenderPass = nullptr;
		GraphicsAPI::RenderPass* lightingRenderPass = nullptr;
		GraphicsAPI::RenderPass* forwardLitRenderPass = nullptr;
		GraphicsAPI::RenderPass* ssaoRenderPass = nullptr;
		GraphicsAPI::RenderPass* ssaoBlurRenderPass = nullptr;
		GraphicsAPI::RenderPass* shadowMapRenderPass = nullptr;
		GraphicsAPI::RenderPass* targetRenderPass = nullptr;
		GraphicsAPI::RenderPass* mainRenderpass = nullptr;
		GraphicsAPI::RenderPass* gbufferRenderpass = nullptr;
	};

	DeferredRendererRenderPasses InitializeRenderPasses();
	void ReleaseRenderPasses(DeferredRendererRenderPasses& rps);
}
