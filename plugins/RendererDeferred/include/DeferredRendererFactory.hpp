#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "DeferredRenderer.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class RenderPass;
	};

	class DeferredRendererFactory : public BaseRendererFactory {
	public:
		DeferredRendererFactory();
		virtual Grindstone::BaseRenderer* CreateRenderer(GraphicsAPI::RenderPass* targetRenderPass) override;
		virtual ~DeferredRendererFactory();


		uint16_t GetRenderModeCount() const;
		const Grindstone::BaseRenderer::RenderMode* GetRenderModes() const;

	private:
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

		static std::array<Grindstone::BaseRenderer::RenderMode, static_cast<uint16_t>(DeferredRenderer::DeferredRenderMode::Count)> renderModes;

	};
}
