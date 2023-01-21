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
			virtual void* ProcessLoadedFile(Uuid uuid) override;
			virtual bool TryGetIfLoaded(Uuid uuid, void*& output) override;
		private:
			std::map<Uuid, ShaderAsset> shaders;
	};
}
