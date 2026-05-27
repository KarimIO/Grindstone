#pragma once

#include <Common/HashedString.hpp>
#include <EngineCore/WorldContext/WorldContext.hpp>
#include <Common/Rendering/RenderGraph.hpp>
#include <Common/Rendering/RenderGraphBuilder.hpp>


namespace Grindstone::Rendering {
	const Grindstone::ConstHashedString renderGraphWorldContextName("Grindstone::Rendering::RenderGraphWorldContext");
	class RenderGraphWorldContext : public Grindstone::WorldContext {
	public:
		RenderGraphWorldContext();
		RenderGraphWorldContext(const RenderGraphWorldContext&) = delete;
		RenderGraphWorldContext(RenderGraphWorldContext&&) noexcept = default;
		virtual ~RenderGraphWorldContext() override = default;

		[[nodiscard]] static RenderGraphWorldContext* GetActiveContext();
		static void SetActiveContext(RenderGraphWorldContext& cxt);
		virtual void SetAsActive() override;

		virtual void StartFrame();
		virtual Grindstone::Renderer::RenderGraph& GetRenderGraph();

	protected:

		Grindstone::Renderer::RenderGraph renderGraph;

	};
}
