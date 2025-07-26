#include <fstream>


#include <Common/ResourcePipeline/MetaFile.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Logger.hpp>
#include <Editor/EditorManager.hpp>
#include <PipelineSet/Converter/PipelineSetConditioner.hpp>

#include "PipelineSetImporter.hpp"

using namespace Grindstone::Editor::Importers;

static void Log(Grindstone::LogSeverity severity, PipelineConverterLogSource source, std::string_view msg, const std::filesystem::path& filename, uint32_t line, uint32_t column) {
	if (severity == Grindstone::LogSeverity::Trace) {
		return;
	}

	if (line == UNDEFINED_LINE && column == UNDEFINED_COLUMN) {
		GPRINT_V(severity, Grindstone::LogSource::EditorImporter, "{} - {}", filename.string(), msg);
		return;
	}

	if (column == UNDEFINED_COLUMN) {
		GPRINT_V(severity, Grindstone::LogSource::EditorImporter, "{}:{} - {}", filename.string(), line, msg);
		return;
	}

	GPRINT_V(severity, Grindstone::LogSource::EditorImporter, "{}:{}:{} - {}", filename.string(), line, column, msg);
}

static std::filesystem::path ResolvePath(Grindstone::Editor::AssetRegistry& assetRegistry, const std::filesystem::path& path) {
	std::filesystem::path outMountedPath;
	if (assetRegistry.TryGetAbsolutePathFromMountedPath(path, outMountedPath)) {
		return outMountedPath;
	}

	return path;
}

void Grindstone::Editor::Importers::ImportShadersFromGlsl(Grindstone::Editor::AssetRegistry& assetRegistry, Grindstone::Assets::AssetManager& assetManager, const std::filesystem::path& filePath) {
	CompilationOptions options;
	options.isDebug = true;
	options.target = CompilationOptions::Target::Vulkan;

	WriteCallback writeCallback = [&assetRegistry, &assetManager] (const std::filesystem::path& path, const std::vector<PipelineOutput>& pipelines) {
		Grindstone::Editor::MetaFile metaFile = assetRegistry.GetMetaFileByPath(path);

		for (size_t index = 0; index < pipelines.size(); ++index) {
			const PipelineOutput& pout = pipelines[index];
			Grindstone::AssetType assetType =
				pout.pipelineType == Grindstone::Formats::Pipelines::V1::PipelineType::Graphics
				? Grindstone::AssetType::GraphicsPipelineSet
				: Grindstone::AssetType::ComputePipelineSet;

			std::string pipelineName = std::string(pout.name);
			Grindstone::Uuid uuid = index == 0
				? metaFile.GetOrCreateDefaultSubassetUuid(pipelineName, assetType)
				: metaFile.GetOrCreateSubassetUuid(pipelineName, assetType);
			std::filesystem::path outputPath = assetRegistry.GetCompiledAssetsPath() / uuid.ToString();

			std::ofstream outStream(outputPath, std::ios::binary);
			if (outStream.fail()) {
				GPRINT_ERROR_V(Grindstone::LogSource::EditorImporter, "{} - Unable to write shader {} to file {}.", path.string(), pout.name, outputPath.string());
				return;
			}

			outStream.write(pout.content.data(), pout.content.size());
			outStream.close();

			// TODO: Maybe queuing all assets at once will improve efficiency?
			assetManager.QueueReloadAsset(assetType, uuid);
		}

		metaFile.Save(pipelineSetImporterVersion);
	};

	ResolvePathCallback resolveCallback = [&assetRegistry](const std::filesystem::path& path) {
		return ResolvePath(assetRegistry, path);
	};

	PipelineSetConditioner conditioner(writeCallback, ::Log, resolveCallback);
	conditioner.Add(filePath);
	conditioner.Convert(options);
}
