#pragma once

#include <glm/mat4x4.hpp>
#include <map>
#include <string>
#include <vector>

#include <Common/Graphics/Core.hpp>
#include <EngineCore/AssetRenderer/BaseAssetRenderer.hpp>

#include "Assets/Mesh3dAsset.hpp"
#include "Components/MeshRendererComponent.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class CommandBuffer;
		class DescriptorSet;
	}

	class EngineCore;

	class Mesh3dRenderer : public BaseAssetRenderer {
		public:
			Mesh3dRenderer(EngineCore* engineCore);

			virtual Grindstone::Rendering::GeometryRenderStats RenderQueue(
				GraphicsAPI::CommandBuffer* commandBuffer,
				Grindstone::Rendering::AssetRenderQueueContext& cxt
			) override;
			static GraphicsAPI::DescriptorSetLayout* GetPerDrawDescriptorSetLayout();
		private:
			virtual std::string GetName() const override;
			virtual void SetEngineDescriptorSet(GraphicsAPI::DescriptorSet* descriptorSet) override;

			EngineCore* engineCore = nullptr;
			std::string rendererName = "Mesh3d";
			GraphicsAPI::DescriptorSet* engineDescriptorSet = nullptr;
			static GraphicsAPI::DescriptorSetLayoutRef perDrawDescriptorSetLayout;
	};
}
