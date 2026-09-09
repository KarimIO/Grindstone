#pragma once

#include <string>

#include <Common/Rendering/RenderFrameContext.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>
#include <Common/Rendering/RenderGraphOrderEvent.hpp>

namespace Grindstone::Rendering {
	class RendererFeature {
	public:
		RendererFeature() = delete;
		RendererFeature(const char* featureName, uint32_t order) : featureName(featureName), renderGraphOrderEvent(order) {}
		RendererFeature(const RendererFeature& o) = delete;
		RendererFeature(RendererFeature&& o) noexcept = delete;
		RendererFeature& operator=(const RendererFeature& o) = delete;
		RendererFeature& operator=(RendererFeature&& o) noexcept = delete;

		virtual void Initialize() {};
		virtual ~RendererFeature() {};
		virtual void Bind(
			Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
			Grindstone::Renderer::RenderFrameContext& context
		) = 0;
		const std::string& GetFeatureName() const { return featureName; }
		uint32_t GetOrder() const { return renderGraphOrderEvent; }
		virtual uint32_t GetRenderGraphOrder() {
			return renderGraphOrderEvent;
		}

	protected:
		std::string featureName = "[[Unnamed]]";
		uint32_t renderGraphOrderEvent = 0;
	};
}
