#pragma once

#include <string>
#include <vector>
#include <map>

#include "../BaseAssetRenderer.hpp"
#include "../Shaders/Shader.hpp"
#include "../Materials/Material.hpp"
#include "Mesh3d.hpp"

namespace Grindstone {
	class Mesh3dRenderer : public BaseAssetRenderer {
		public:
			Mesh3dRenderer();
			void RenderShader(Shader& shader);
			void RenderMaterial(Material& material);
			void RenderSubmesh(Mesh3d::Submesh& submesh3d);
		private:
			virtual void RenderQueue(RenderQueueContainer& renderQueue) override;
	};
}
