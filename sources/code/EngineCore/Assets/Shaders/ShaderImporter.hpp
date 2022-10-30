#pragma once

#include <string>
#include <map>
#include <fstream>

#include "EngineCore/Assets/AssetImporter.hpp"
#include "ShaderAsset.hpp"

namespace Grindstone {
	class BaseAssetRenderer;
	class ShaderImporter : public AssetImporter {
		public:
			virtual void Load(Uuid uuid) override;
	};
}
