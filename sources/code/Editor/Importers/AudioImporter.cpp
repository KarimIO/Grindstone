#include "AudioImporter.hpp"
#include "Common/ResourcePipeline/MetaFile.hpp"
#include "Common/ResourcePipeline/Uuid.hpp"
#include "Editor/EditorManager.hpp"

using namespace Grindstone;
using namespace Grindstone::Importers;

void AudioImporter::Import(std::filesystem::path& path) {
	metaFile = new MetaFile(path);
	std::string subassetName = path.filename().string();
	size_t dotPos = subassetName.find('.');
	if (dotPos != std::string::npos) {
		subassetName = subassetName.substr(0, dotPos);
	}
	Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName, AssetType::AudioClip);

	std::filesystem::path outputPath = Editor::Manager::GetInstance().GetCompiledAssetsPath() / uuid.ToString();
	std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
	metaFile->Save();
}

void Importers::ImportAudio(std::filesystem::path& inputPath) {
	AudioImporter audioImporter;
	audioImporter.Import(inputPath);
}
