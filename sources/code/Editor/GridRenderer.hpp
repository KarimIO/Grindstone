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
		void Initialize(class Grindstone::GraphicsAPI::RenderPass* renderPass);
		void Render(Grindstone::GraphicsAPI::CommandBuffer* commandBuffer, glm::vec2 renderScale, glm::mat4 proj, glm::mat4 view, float nearDist, float farDist, glm::quat rotation, float offset);
	protected:
		Grindstone::AssetReference<Grindstone::GraphicsPipelineAsset> pipelineSet;
		Grindstone::GraphicsAPI::Buffer* gridUniformBuffer = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* gridDescriptorSet = nullptr;
		Grindstone::GraphicsAPI::DescriptorSetLayout* gridDescriptorSetLayout = nullptr;
	};
}
