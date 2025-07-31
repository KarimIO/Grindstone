#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#include <EngineCore/Assets/PipelineSet/GraphicsPipelineAsset.hpp>
#include <EngineCore/Assets/AssetReference.hpp>

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class Buffer;
	class DescriptorSet;
	class DescriptorSetLayout;
	class GraphicsPipeline;
	class RenderPass;
}

namespace Grindstone::Editor {
	class GridRenderer {
	public:
		void Initialize();
		void Render(Grindstone::GraphicsAPI::CommandBuffer* commandBuffer, glm::vec2 renderScale, glm::mat4 proj, glm::mat4 view, float nearDist, float farDist, glm::quat rotation, float offset);
	protected:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> pipelineSet;
		std::array<Grindstone::GraphicsAPI::Buffer*, 3> gridUniformBuffers = {};
		std::array<Grindstone::GraphicsAPI::DescriptorSet*, 3> gridDescriptorSets = {};
		Grindstone::GraphicsAPI::DescriptorSetLayout* gridDescriptorSetLayout = nullptr;
	};
}
