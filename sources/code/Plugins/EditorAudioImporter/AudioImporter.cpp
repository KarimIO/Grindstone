#include <filesystem>

#include <Common/ResourcePipeline/MetaFile.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Editor/EditorManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

#include "AudioImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

void Editor::Importers::ImportAudio(AssetRegistry& assetRegistry, Assets::AssetManager& assetManager, const std::filesystem::path& inputPath) {
	Grindstone::Editor::MetaFile metaFile = assetRegistry.GetMetaFileByPath(inputPath);
	std::string subassetName = inputPath.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}
	Uuid uuid = metaFile.GetOrCreateDefaultSubassetUuid(subassetName, AssetType::AudioClip);
	metaFile.Save(audioImporterVersion);

	std::filesystem::path outputPath = assetRegistry.GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(inputPath, outputPath, std::filesystem::copy_options::overwrite_existing);

	assetManager.QueueReloadAsset(AssetType::AudioClip, uuid);
}
