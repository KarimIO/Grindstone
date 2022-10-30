#pragma once

#include <filesystem>
#include <string>
#include <map>

#include "EngineCore/Assets/AssetImporter.hpp"
#include "MaterialAsset.hpp"

namespace Grindstone {
	class BaseAssetRenderer;

	class MaterialImporter : public AssetImporter {
		public:
			virtual void Load(Uuid uuid) override;
	};
}
