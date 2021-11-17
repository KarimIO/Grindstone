#include "AudioImporter.hpp"
#include "Common/ResourcePipeline/MetaFile.hpp"
#include "Common/ResourcePipeline/Uuid.hpp"

using namespace Grindstone;
using namespace Grindstone::Importers;

void AudioImporter::Import(std::filesystem::path& path) {
	metaFile = new MetaFile(path);
	std::string subassetName = "audioClip";
	Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(subassetName);

	std::filesystem::copy(path, std::string("../compiledAssets/") + uuid.ToString(), std::filesystem::copy_options::overwrite_existing);
	metaFile->Save();
}

void Importers::ImportAudio(std::filesystem::path& inputPath) {
	AudioImporter audioImporter;
	audioImporter.Import(inputPath);
}
