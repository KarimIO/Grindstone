#pragma once

#include <string>
#include <vector>
#include <map>

#include "../BaseAssetRenderer.hpp"
#include "../Shaders/Shader.hpp"
#include "../Materials/Material.hpp"
#include "Mesh3d.hpp"

namespace Grindstone {
	namespace GraphicsAPI {
		class UniformBufferBinding;
		class UniformBuffer;
	}

	class Mesh3dRenderer : public BaseAssetRenderer {
		public:
			Mesh3dRenderer();
			void AddErrorMaterial();
			void RenderShader(Shader& shader);
			void RenderMaterial(Material& material);
			void RenderSubmesh(ECS::Entity entity, Mesh3d::Submesh& submesh3d);
		private:
			virtual void RenderQueue(RenderQueueContainer& renderQueue) override;

			GraphicsAPI::UniformBufferBinding* mesh3dBufferBinding = nullptr;
			GraphicsAPI::UniformBuffer* mesh3dBufferObject = nullptr;;
	};
}
