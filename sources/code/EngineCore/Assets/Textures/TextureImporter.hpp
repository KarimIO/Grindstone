#pragma once

#include <string>
#include <vector>
#include <map>

#include "TextureAsset.hpp"
#include "EngineCore/Assets/AssetImporter.hpp"

namespace Grindstone {
	class TextureImporter : public AssetImporter {
	public:
		virtual void* ProcessLoadedFile(Uuid uuid, char* fileContents, size_t fileSize) override;
		virtual bool TryGetIfLoaded(Uuid uuid, void*& output) override;
	private:
		std::map<Uuid, TextureAsset> textures;
	};
}
