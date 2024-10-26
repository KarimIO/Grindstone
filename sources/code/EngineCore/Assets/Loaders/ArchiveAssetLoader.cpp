#include <fstream>
#include <filesystem>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>
#include <Common/Graphics/Core.hpp>

#include "ArchiveDirectoryDeserializer.hpp"
#include "ArchiveAssetLoader.hpp"
using namespace Grindstone::Assets;

ArchiveAssetLoader::ArchiveAssetLoader() {
	InitializeDirectory();
}

void ArchiveAssetLoader::InitializeDirectory() {
	ArchiveDirectoryDeserializer deserializer(archiveDirectory);
}

AssetLoadResult ArchiveAssetLoader::Load(AssetType assetType, Uuid uuid, std::string& assetName) {
	size_t assetTypeIndex = static_cast<size_t>(assetType);

	if (assetTypeIndex >= static_cast<size_t>(AssetType::Count)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Invalid Asset Type when trying to load file: {}", uuid.ToString());
		return { AssetLoadStatus::InvalidAssetType, {} };
	}

	ArchiveDirectory::AssetTypeIndex& assetTypeSegment = archiveDirectory.assetTypeIndices[static_cast<size_t>(assetType)];
	const auto& assetIterator = assetTypeSegment.assets[uuid];

	if (assetIterator.size == 0) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not load asset: {}", uuid.ToString());
		return { AssetLoadStatus::InvalidAssetType, {} };
	}

	return LoadAsset(assetIterator, assetName);
}

AssetLoadResult ArchiveAssetLoader::Load(AssetType assetType, std::filesystem::path path, std::string& assetName) {
	size_t assetTypeIndex = static_cast<size_t>(assetType);

	if (assetTypeIndex >= static_cast<size_t>(AssetType::Count)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Invalid Asset Type when trying to load file: ", path.string());
		return { AssetLoadStatus::InvalidAssetType, {} };
	}

	ArchiveDirectory::AssetInfo* assetInfo = nullptr;
	ArchiveDirectory::AssetTypeIndex& assetTypeSegment = archiveDirectory.assetTypeIndices[static_cast<size_t>(assetType)];
	for (auto& asset : assetTypeSegment.assets) {
		if (asset.second.filename == path) {
			assetInfo = &asset.second;
		}
	}

	if (assetInfo == nullptr) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not load asset: {}", path.string());
		return { AssetLoadStatus::InvalidAssetType, {} };
	}

	return LoadAsset(*assetInfo, assetName);
}

bool ArchiveAssetLoader::LoadText(AssetType assetType, Uuid uuid, std::string& assetName, std::string& outContents) {
	AssetLoadResult result = Load(assetType, uuid, assetName);

	if (result.status != AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not load asset: {}", uuid.ToString());
		return false;
	}

	outContents = reinterpret_cast<const char*>(result.buffer.Get());
	return true;
}

bool ArchiveAssetLoader::LoadText(AssetType assetType, std::filesystem::path path, std::string& assetName, std::string& outContents) {
	AssetLoadResult result = Load(assetType, path, assetName);

	if (result.status != AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not load asset: {}", path.string());
		return false;
	}

	outContents = reinterpret_cast<const char*>(result.buffer.Get());
	return true;
}

AssetLoadResult ArchiveAssetLoader::LoadAsset(const ArchiveDirectory::AssetInfo& assetInfo, std::string& assetName) {
	if (lastBufferIndex != assetInfo.archiveIndex) {
		lastBufferIndex = assetInfo.archiveIndex;

		const std::string filename = "TestArchive_0.garc";
		const std::filesystem::path path = EngineCore::GetInstance().GetProjectPath() / "archives" / filename;
		const std::string filepathAsStr = path.string();
		lastBuffer = Utils::LoadFile(filepathAsStr.c_str());
	}

	assetName = assetInfo.filename;
	return { AssetLoadStatus::Success, Buffer(lastBuffer.Get() + assetInfo.offset, assetInfo.size) };
}

bool ArchiveAssetLoader::LoadShaderStage(
	const Uuid uuid,
	const GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	std::string path = GetShaderPath(uuid, shaderStage);
	stageCreateInfo.fileName = path.c_str();

	if (!std::filesystem::exists(path)) {
		std::string errorMsg = path + " shader not found.";
		GPRINT_ERROR(LogSource::EngineCore, errorMsg.c_str());
		return false;
	}

	Buffer buffer = Utils::LoadFile(path.c_str());
	fileData.resize(buffer.GetCapacity());
	memcpy(fileData.data(), buffer.Get(), buffer.GetCapacity());
	stageCreateInfo.content = fileData.data();
	stageCreateInfo.size = static_cast<uint32_t>(fileData.size());
	stageCreateInfo.type = shaderStage;

	return true;
}

std::string ArchiveAssetLoader::GetShaderPath(
	Uuid uuid,
	GraphicsAPI::ShaderStage shaderStage
) {
	const char* shaderStageExtension = "";

	switch (shaderStage) {
	case GraphicsAPI::ShaderStage::Vertex:
		shaderStageExtension = ".vert";
		break;
	case GraphicsAPI::ShaderStage::Fragment:
		shaderStageExtension = ".frag";
		break;
	case GraphicsAPI::ShaderStage::TesselationEvaluation:
		shaderStageExtension = ".eval";
		break;
	case GraphicsAPI::ShaderStage::TesselationControl:
		shaderStageExtension = ".ctrl";
		break;
	case GraphicsAPI::ShaderStage::Geometry:
		shaderStageExtension = ".geom";
		break;
	case GraphicsAPI::ShaderStage::Compute:
		shaderStageExtension = ".comp";
		break;
	default:
		GPRINT_ERROR(LogSource::EngineCore, "Incorrect shader stage");
		break;
	}

	std::filesystem::path path = EngineCore::GetInstance().GetAssetPath(uuid.ToString());
	return path.string() + shaderStageExtension + EngineCore::GetInstance().GetGraphicsCore()->GetDefaultShaderExtension();
}
