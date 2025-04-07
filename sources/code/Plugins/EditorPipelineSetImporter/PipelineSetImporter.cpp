#include <fstream>

#include <PipelineSet/Converter/PipelineSetConditioner.hpp>

#include <Common/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Logger.hpp>
#include <Editor/EditorManager.hpp>

#include "PipelineSetImporter.hpp"

using namespace Grindstone::Editor::Importers;

static void Log(LogLevel level, LogSource source, std::string_view msg, const std::filesystem::path& filename, uint32_t line, uint32_t column) {
	GPRINT_V(static_cast<Grindstone::LogSeverity>(level), Grindstone::LogSource::EditorImporter, "{}:{}:{} - {}", filename.string(), line, column, msg);
}

void Grindstone::Editor::Importers::ImportShadersFromGlsl(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& filePath) {
	CompilationOptions options;
	options.isDebug = true;
	options.target = CompilationOptions::Target::Vulkan;

	WriteCallback writeCallback = [&assetRegistry, &assetManager] (std::filesystem::path path, size_t size, void* content) {
		Grindstone::Editor::MetaFile* metaFile = assetRegistry.GetMetaFileByPath(path);
		Grindstone::Uuid uuid = metaFile->GetOrCreateDefaultSubassetUuid(path.string(), AssetType::Material);

		std::ofstream outStream(path, std::ios::binary);
		if (outStream.is_open()) {
			GPRINT_ERROR_V(Grindstone::LogSource::EditorImporter, "{} - Unable to write file to {}.", path.string(), uuid.ToString());
			return;
		}

		outStream.write(reinterpret_cast<char*>(content), size);
		outStream.close();

		std::filesystem::path outputPath = assetRegistry.GetCompiledAssetsPath() / uuid.ToString();
		std::filesystem::copy(path, outputPath, std::filesystem::copy_options::overwrite_existing);
		metaFile->Save();
		assetManager.QueueReloadAsset(AssetType::Material, uuid);
	};

	PipelineSetConditioner conditioner(writeCallback, ::Log);
	conditioner.Add(filePath);
	conditioner.Convert(options);
}
