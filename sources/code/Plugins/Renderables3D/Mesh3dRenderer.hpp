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
		class Core;
		class UniformBufferBinding;
		class UniformBuffer;
	}

	class Mesh3dRenderer : public BaseAssetRenderer {
		public:
			Mesh3dRenderer();
			void RenderShader(ShaderAsset& shader);
			void RenderMaterial(MaterialAsset& material);
			void RenderSubmesh(ECS::Entity entity, Mesh3dAsset::Submesh& submesh3d);

			static GraphicsAPI::Core* graphicsCore;
		private:
			virtual void RenderQueue(RenderQueueContainer& renderQueue) override;

			GraphicsAPI::UniformBufferBinding* mesh3dBufferBinding = nullptr;
			GraphicsAPI::UniformBuffer* mesh3dBufferObject = nullptr;
	};
}
