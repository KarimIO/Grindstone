#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/mat4x4.hpp>

#include "EngineCore/AssetRenderer/BaseAssetRenderer.hpp"
#include "Components/MeshRendererComponent.hpp"
#include "Assets/Mesh3dAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

	class EngineCore;

	class Mesh3dRenderer : public BaseAssetRenderer {
		public:
			Mesh3dRenderer(EngineCore* engineCore);

			virtual void RenderQueue(
				GraphicsAPI::CommandBuffer* commandBuffer,
				const entt::registry& registry,
				Grindstone::HashedString passType,
				Grindstone::HashedString drawOrderBucket
			) override;
			GraphicsAPI::DescriptorSetLayout* GetPerDrawDescriptorSetLayout() const;
		private:
			virtual std::string GetName() const override;
			virtual void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) override;

			EngineCore* engineCore = nullptr;
			std::string rendererName = "Mesh3d";
			GraphicsAPI::DescriptorSet* engineDescriptorSet = nullptr;
			class GraphicsAPI::DescriptorSetLayout* perDrawDescriptorSetLayout = nullptr;
	};
}
