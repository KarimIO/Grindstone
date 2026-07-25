#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/mat4x4.hpp>

#include <Common/Rendering/RenderViewData.hpp>
#include "EngineCore/AssetRenderer/BaseAssetRenderer.hpp"
#include "Components/MeshRendererComponent.hpp"
#include "Assets/Mesh3dAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

	class EngineCore;

	class SkeletalMeshRenderer : public BaseAssetRenderer {
		public:
			SkeletalMeshRenderer(EngineCore* engineCore);

			virtual Grindstone::Rendering::GeometryRenderStats RenderQueue(
				GraphicsAPI::CommandBuffer* commandBuffer,
				const Grindstone::Rendering::RenderViewData& viewData,
				entt::registry& registry,
				Grindstone::HashedString renderQueueHash
			) override;
			static GraphicsAPI::DescriptorSetLayout* GetPerDrawDescriptorSetLayout();
		private:
			virtual std::string GetName() const override;
			virtual void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) override;

			EngineCore* engineCore = nullptr;
			std::string rendererName = "SkeletalMesh";
			GraphicsAPI::DescriptorSet* engineDescriptorSet = nullptr;
			static class GraphicsAPI::DescriptorSetLayout* perDrawDescriptorSetLayout;
	};
}
