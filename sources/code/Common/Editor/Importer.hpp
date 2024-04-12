#pragma once

#include <filesystem>

#include <Editor/AssetRegistry.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

namespace Grindstone::Editor {
	class MetaFile;

	namespace Importers{
		class Importer {
		public:
			virtual void Import(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& path) = 0;
			~Importer();

		protected:
			MetaFile* metaFile = nullptr;
		};
	}
}
