#pragma once

#include <string>
#include <vector>
#include <map>

#include "EngineCore/AssetRenderer/BaseAssetRenderer.hpp"
#include "EngineCore/Assets/Shaders/ShaderAsset.hpp"
#include "EngineCore/Assets/Materials/MaterialAsset.hpp"
#include "Assets/Mesh3dAsset.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBuffer;
		class CommandBuffer;
	}

	class EngineCore;

	class Mesh3dRenderer : public BaseAssetRenderer {
		public:
			Mesh3dRenderer(EngineCore* engineCore);
			void RenderShaderImmediate(ShaderAsset& shader);
			void RenderMaterialImmediate(MaterialAsset& material);
			void RenderSubmeshImmediate(ECS::Entity entity, Mesh3dAsset::Submesh& submesh3d);
			void RenderShader(GraphicsAPI::CommandBuffer* commandBuffer, ShaderAsset& shader);
			void RenderMaterial(GraphicsAPI::CommandBuffer* commandBuffer, MaterialAsset& material);
			void RenderSubmesh(GraphicsAPI::CommandBuffer* commandBuffer, ECS::Entity entity, Mesh3dAsset::Submesh& submesh3d);

		private:
			virtual void RenderQueueImmediate(RenderQueueContainer& renderQueue) override;
			virtual void RenderQueue(GraphicsAPI::CommandBuffer* commandBuffer, RenderQueueContainer& renderQueue) override;

			EngineCore* engineCore;
			GraphicsAPI::UniformBuffer* mesh3dBufferObject = nullptr;
	};
}
