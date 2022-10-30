#pragma once

#include <string>
#include <vector>
#include <map>

#include "Texture.hpp"
#include "EngineCore/Assets/AssetImporter.hpp"

namespace Grindstone {
	class TextureImporter : public AssetImporter {
		public:
			virtual void Load(Uuid uuid) override;
	};
}
