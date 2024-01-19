#pragma once

#include <string>
#include <map>
#include <fstream>

#include "EngineCore/Assets/AssetImporter.hpp"
#include "ShaderAsset.hpp"

namespace Grindstone {
	class BaseAssetRenderer;
	class ShaderImporter : public SpecificAssetImporter<ShaderAsset, AssetType::Shader> {
		public:
			virtual void* ProcessLoadedFile(Uuid uuid) override;
			virtual void QueueReloadAsset(Uuid uuid) override;
		private:
			bool ImportShader(ShaderAsset& shaderAsset);

	};
}
