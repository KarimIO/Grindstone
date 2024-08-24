#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace Grindstone::GraphicsAPI {
	class CommandBuffer;
	class UniformBuffer;
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
		Grindstone::GraphicsAPI::UniformBuffer* gridUniformBuffer = nullptr;
		Grindstone::GraphicsAPI::DescriptorSet* gridDescriptorSet = nullptr;
		Grindstone::GraphicsAPI::DescriptorSetLayout* gridDescriptorSetLayout = nullptr;
		Grindstone::GraphicsAPI::GraphicsPipeline* gridPipeline = nullptr;
	};
}
