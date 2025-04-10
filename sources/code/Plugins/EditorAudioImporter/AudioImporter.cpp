#include <filesystem>

#include <Common/ResourcePipeline/MetaFile.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <Editor/EditorManager.hpp>
#include <EngineCore/Assets/AssetManager.hpp>

#include "AudioImporter.hpp"

using namespace Grindstone;
using namespace Grindstone::Editor::Importers;

void AudioImporter::Import(AssetRegistry& assetRegistry, Assets::AssetManager& assetManager, const std::filesystem::path& path) {
	metaFile = assetRegistry.GetMetaFileByPath(path);
	std::string subassetName = path.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}
	Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, AssetType::AudioClip);

	std::filesystem::path outputPath = assetRegistry.GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile->Save(audioImporterVersion);

	assetManager.QueueReloadAsset(AssetType::AudioClip, uuid);
}

void Editor::Importers::ImportAudio(AssetRegistry& assetRegistry, Assets::AssetManager& assetManager, const std::filesystem::path& inputPath) {
	AudioImporter audioImporter;
	audioImporter.Import(assetRegistry, assetManager, inputPath);
}
