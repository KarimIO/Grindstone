#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/EngineCore.hpp>
#include <Common/Graphics/Core.hpp>

void Grindstone::Renderer::RenderGraph::Print() {
}

inline void GraphBuilder::SortPasses() {
	const uint32_t numPasses = static_cast<uint32_t>(m_graph->Passes.size());

	// Prepare for topological sort.
	std::vector<std::vector<int>> passes;
	passes.reserve(numPasses);
	for (int i = 0; i < static_cast<int>(numPasses); i++)
	{
		if (m_graph->Passes[i]->m_dependencies.empty()) continue;
		std::vector<int> deps;
		for (int j = 0; j < m_graph->Passes[i]->m_dependencies.size(); j++)
		{
			passes.push_back({ m_graph->PassTypes[m_graph->Passes[i]->m_dependencies[j]], i });
		}
	}

	// Do topological sort.
	auto sortedIndices = Utils::TopologicalSortKahn(numPasses, passes);

	// Add the sorted passes to the Graph.
	m_graph->SortedPasses.reserve(numPasses);
	for (uint32_t i = 0; i < numPasses; i++)
	{
		m_graph->SortedPasses.push_back(m_graph->Passes[sortedIndices[i]]);
	}
}

void Grindstone::Renderer::RenderGraph::ExecuteGraph(Grindstone::Renderer::RenderGraph::RenderGraphContext context) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::GraphicsAPI::Core* core = engineCore.GetGraphicsCore();
	Grindstone::GraphicsAPI::CommandBuffer* cmd = context.commandBuffer;

	for (Grindstone::Renderer::RenderGraph::RenderPass& pass : sortedPasses) {
		if (!pass->barriers_.image_barriers.empty() ||
			!pass->barriers_.buffer_barriers.empty()) {
			vkCmdPipelineBarrier(
				cmd,
				pass->barriers_.src_stage,
				pass->barriers_.dst_stage,
				0,
				0, nullptr,
				pass->barriers_.buffer_barriers.size(),
				pass->barriers_.buffer_barriers.data(),
				pass->barriers_.image_barriers.size(),
				pass->barriers_.image_barriers.data()
			);
		}

		cmd->BeginRendering(
			"Pass Name",
			context.swapchainSize,
			pass.colorAttachments.data(),
			pass.colorAttachments.size(),
			pass.depthAttachment
		);

		Grindstone::Renderer::RenderGraph::RenderPassExecution renderGraphExecution;
		pass.execute(context, renderGraphExecution);

		cmd->EndRendering();
	}
}
