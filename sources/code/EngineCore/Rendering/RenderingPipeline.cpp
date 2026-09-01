#include <Common/Graphics/WindowGraphicsBinding.hpp>
#include <EngineCore/EngineCore.hpp>

#include "RenderingPipeline.hpp"

using namespace Grindstone::Renderer;

std::array<RenderMode, 13> renderModes = {
	RenderMode{ "Default" },
	RenderMode{ "World Position" },
	RenderMode{ "World Position (Modulus)" },
	RenderMode{ "View Position" },
	RenderMode{ "View Position (Modulus)" },
	RenderMode{ "Depth" },
	RenderMode{ "Depth (Modulus)" },
	RenderMode{ "Normals" },
	RenderMode{ "View Normals" },
	RenderMode{ "Albedo" },
	RenderMode{ "Specular" },
	RenderMode{ "Roughness" },
	RenderMode{ "Ambient Occlusion" }
};

void RenderingPipeline::Render(
	Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
	Grindstone::Renderer::RenderFrameContext& renderFrameContext
) {
	renderStats.clear();
	for (Grindstone::Rendering::RendererFeature* feature : features) {
		feature->Bind(renderGraphBuilder, renderFrameContext);
	}
}

std::vector<Grindstone::Rendering::GeometryRenderStats> Grindstone::Renderer::RenderingPipeline::GetRenderingStats() {
	return renderStats;
}

void Grindstone::Renderer::RenderingPipeline::PushRenderingStats(const Grindstone::Rendering::GeometryRenderStats& stats) {
	renderStats.emplace_back(stats);
}

static bool RendererFeatureOrder(const Grindstone::Rendering::RendererFeature* lhs, const Grindstone::Rendering::RendererFeature* rhs) {
	return (lhs->GetOrder() < rhs->GetOrder());
}

void Grindstone::Renderer::RenderingPipeline::RegisterFeature(Grindstone::Rendering::RendererFeature* feature) {
	const std::string& featureName = feature->GetFeatureName();
	GS_ASSERT(std::find_if(features.begin(), features.end(), [&featureName](Grindstone::Rendering::RendererFeature* f) { return f->GetFeatureName() == featureName; }) == features.end());

	// Insert sorted
	auto it = std::lower_bound(features.begin(), features.end(), feature, RendererFeatureOrder);
	features.insert(it, feature);
	feature->Initialize();
}

void Grindstone::Renderer::RenderingPipeline::UnregisterFeature(const char* featureName) {
	auto it = std::find_if(features.begin(), features.end(), [featureName](Grindstone::Rendering::RendererFeature* f) { return f->GetFeatureName() == featureName; });
	GS_ASSERT(it != features.end());
	features.erase(it);
}
