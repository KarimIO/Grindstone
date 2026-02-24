#pragma once

#include <Common/Rendering/RenderGraph.hpp>
#include <EngineCore/Assets/AssetReference.hpp>
#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>

namespace Grindstone::GraphicsAPI {
	class DescriptorSetLayout;
	class DescriptorSet;
	class Buffer;
}

namespace Grindstone::Renderer {
	struct PostProcessSettings {
		glm::vec4 vignetteColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		float vignetteRadius = 0.75f;
		float vignetteSoftness = 0.8f;
		float grainAmount = 0.0007f;
		float grainPixelSize = 1.0f;
		glm::vec2 chromaticDistortionRedOffset = glm::vec2(0.00045f, 0.00045f);
		glm::vec2 chromaticDistortionGreenOffset = glm::vec2(0.0003f, 0.0003f);
		glm::vec2 chromaticDistortionBlueOffset = glm::vec2(-0.0003f, -0.0003f);
		float paniniDistortionStrength = 0.0f;
		bool isAnimated = true;
	};

	class TonemapPass {
	public:
		bool Initialize();
		void AddPass(Grindstone::Renderer::RenderGraph& renderGraph, PostProcessSettings settings);

	private:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> tonemapPipelineSet;
		std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> tonemapSettingsDescriptorSet {};
		std::array<Grindstone::GraphicsAPI::Buffer*, 3> tonemapSettingsUniformBuffer {};
		GraphicsAPI::DescriptorSetLayout* descriptorSetLayout = nullptr;
	};
}
