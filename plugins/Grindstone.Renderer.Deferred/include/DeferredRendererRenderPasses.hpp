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
		GraphicsAPI::RenderPass* editorRenderPass = nullptr;
		GraphicsAPI::RenderPass* gizmoRenderPass = nullptr;
		GraphicsAPI::RenderPass* mousePickRenderPass = nullptr;
		GraphicsAPI::RenderPass* selectionSystemRenderPass = nullptr;
	};

	DeferredRendererRenderPasses InitializeRenderPasses();
	void ReleaseRenderPasses(DeferredRendererRenderPasses& rps);
}
