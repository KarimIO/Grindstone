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

AssetLoadBinaryResult ArchiveAssetLoader::LoadBinaryByPath(AssetType assetType, const std::filesystem::path& path) {
	size_t assetTypeIndex = static_cast<size_t>(assetType);

	if (assetTypeIndex >= static_cast<size_t>(AssetType::Count)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Invalid Asset Type when trying to load file: {}", path.string());
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

	return LoadAsset(*assetInfo);
}

AssetLoadBinaryResult ArchiveAssetLoader::LoadBinaryByAddress(AssetType assetType, std::string_view address) {
	size_t assetTypeIndex = static_cast<size_t>(assetType);

	if (assetTypeIndex >= static_cast<size_t>(AssetType::Count)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Invalid Asset Type when trying to load file: {}", address);
		return { AssetLoadStatus::InvalidAssetType, {} };
	}

	ArchiveDirectory::AssetInfo* assetInfo = nullptr;
	ArchiveDirectory::AssetTypeIndex& assetTypeSegment = archiveDirectory.assetTypeIndices[static_cast<size_t>(assetType)];
	for (auto& asset : assetTypeSegment.assets) {
		if (asset.second.filename == address) {
			assetInfo = &asset.second;
		}
	}

	if (assetInfo == nullptr) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not load asset: {}", address);
		return { AssetLoadStatus::InvalidAssetType, {} };
	}

	return LoadAsset(*assetInfo);
}

AssetLoadBinaryResult ArchiveAssetLoader::LoadBinaryByUuid(AssetType assetType, Uuid uuid) {
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

	return LoadAsset(assetIterator);
}



AssetLoadTextResult ArchiveAssetLoader::LoadTextByPath(AssetType assetType, const std::filesystem::path& path) {
	AssetLoadBinaryResult result = LoadBinaryByPath(assetType, path);
	return {
		result.status,
		result.displayName,
		std::string(reinterpret_cast<const char*>(result.buffer.Get()))
	};
}

AssetLoadTextResult ArchiveAssetLoader::LoadTextByAddress(AssetType assetType, std::string_view address) {
	AssetLoadBinaryResult result = LoadBinaryByAddress(assetType, address);
	return {
		result.status,
		result.displayName,
		std::string(reinterpret_cast<const char*>(result.buffer.Get()))
	};
}

AssetLoadTextResult ArchiveAssetLoader::LoadTextByUuid(AssetType assetType, Uuid uuid) {
	AssetLoadBinaryResult result = LoadBinaryByUuid(assetType, uuid);
	return {
		result.status,
		result.displayName,
		std::string(reinterpret_cast<const char*>(result.buffer.Get()))
	};
}

AssetLoadBinaryResult ArchiveAssetLoader::LoadAsset(const ArchiveDirectory::AssetInfo& assetInfo) {
	if (lastBufferIndex != assetInfo.archiveIndex) {
		lastBufferIndex = assetInfo.archiveIndex;

		const std::string filename = "TestArchive_0.garc";
		const std::filesystem::path path = EngineCore::GetInstance().GetProjectPath() / "archives" / filename;
		const std::string filepathAsStr = path.string();
		lastBuffer = Utils::LoadFile(filepathAsStr.c_str());
	}

	return { AssetLoadStatus::Success, std::string(assetInfo.filename), Buffer(lastBuffer.Get() + assetInfo.offset, assetInfo.size) };
}

static std::filesystem::path GetShaderPath(
	const std::filesystem::path& path,
	Grindstone::GraphicsAPI::ShaderStage shaderStage
) {
	const char* shaderStageExtension = "";

	switch (shaderStage) {
	case Grindstone::GraphicsAPI::ShaderStage::Vertex:
		shaderStageExtension = ".vert";
		break;
	case Grindstone::GraphicsAPI::ShaderStage::Fragment:
		shaderStageExtension = ".frag";
		break;
	case Grindstone::GraphicsAPI::ShaderStage::TesselationEvaluation:
		shaderStageExtension = ".eval";
		break;
	case Grindstone::GraphicsAPI::ShaderStage::TesselationControl:
		shaderStageExtension = ".ctrl";
		break;
	case Grindstone::GraphicsAPI::ShaderStage::Geometry:
		shaderStageExtension = ".geom";
		break;
	case Grindstone::GraphicsAPI::ShaderStage::Compute:
		shaderStageExtension = ".comp";
		break;
	default:
		GPRINT_ERROR(Grindstone::LogSource::Editor, "Incorrect shader stage");
		break;
	}

	const char* shaderExt = Grindstone::EngineCore::GetInstance().GetGraphicsCore()->GetDefaultShaderExtension();
	return path.string() + shaderStageExtension + shaderExt;
}

bool ArchiveAssetLoader::LoadShaderStageByPath(
	const std::filesystem::path& path,
	const GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	EngineCore& engineCore = EngineCore::GetInstance();
	std::filesystem::path fullPath = engineCore.GetAssetPath(path.string());
	std::filesystem::path stagePath = GetShaderPath(fullPath, shaderStage);
	stageCreateInfo.fileName = stagePath.string().c_str();

	if (!std::filesystem::exists(stagePath)) {
		std::string errorMsg = stagePath.string() + " shader not found.";
		GPRINT_ERROR(LogSource::Editor, errorMsg.c_str());
		return false;
	}

	Buffer buffer = Utils::LoadFile(stagePath.string().c_str());
	fileData.resize(buffer.GetCapacity());
	memcpy(fileData.data(), buffer.Get(), buffer.GetCapacity());
	stageCreateInfo.content = fileData.data();
	stageCreateInfo.size = static_cast<uint32_t>(fileData.size());
	stageCreateInfo.type = shaderStage;

	return true;
}

bool ArchiveAssetLoader::LoadShaderStageByAddress(
	std::string_view address,
	const GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	EngineCore& engineCore = EngineCore::GetInstance();
	std::filesystem::path path = engineCore.GetAssetPath(std::string(address));
	std::filesystem::path stagePath = GetShaderPath(path, shaderStage);
	stageCreateInfo.fileName = stagePath.string().c_str();

	if (!std::filesystem::exists(stagePath)) {
		std::string errorMsg = stagePath.string() + " shader not found.";
		GPRINT_ERROR(LogSource::Editor, errorMsg.c_str());
		return false;
	}

	Buffer buffer = Utils::LoadFile(stagePath.string().c_str());
	fileData.resize(buffer.GetCapacity());
	memcpy(fileData.data(), buffer.Get(), buffer.GetCapacity());
	stageCreateInfo.content = fileData.data();
	stageCreateInfo.size = static_cast<uint32_t>(fileData.size());
	stageCreateInfo.type = shaderStage;

	return true;
}

bool ArchiveAssetLoader::LoadShaderStageByUuid(
	Uuid uuid,
	const GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	EngineCore& engineCore = EngineCore::GetInstance();
	std::filesystem::path path = engineCore.GetAssetPath(uuid.ToString());
	std::filesystem::path stagePath = GetShaderPath(path, shaderStage);
	stageCreateInfo.fileName = stagePath.string().c_str();

	if (!std::filesystem::exists(stagePath)) {
		std::string errorMsg = stagePath.string() + " shader not found.";
		GPRINT_ERROR(LogSource::Editor, errorMsg.c_str());
		return false;
	}

	Buffer buffer = Utils::LoadFile(stagePath.string().c_str());
	fileData.resize(buffer.GetCapacity());
	memcpy(fileData.data(), buffer.Get(), buffer.GetCapacity());
	stageCreateInfo.content = fileData.data();
	stageCreateInfo.size = static_cast<uint32_t>(fileData.size());
	stageCreateInfo.type = shaderStage;

	return true;
}
