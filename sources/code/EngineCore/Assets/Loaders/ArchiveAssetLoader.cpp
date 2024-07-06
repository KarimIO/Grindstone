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

// Out:
//	- outContents should be nullptr
//	- fileSize should be 0
void ArchiveAssetLoader::Load(AssetType assetType, Uuid uuid, std::string& assetName, char*& outContents, size_t& fileSize) {
	size_t assetTypeIndex = static_cast<size_t>(assetType);

	if (assetTypeIndex >= static_cast<size_t>(AssetType::Count)) {
		outContents = nullptr;
		fileSize = 0;

		std::string errorString = "Invalid Asset Type when trying to load file: " + uuid.ToString();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return;
	}

	ArchiveDirectory::AssetTypeIndex& assetTypeSegment = archiveDirectory.assetTypeIndices[static_cast<size_t>(assetType)];
	const auto& assetIterator = assetTypeSegment.assets[uuid];

	if (assetIterator.size == 0) {
		outContents = nullptr;
		fileSize = 0;

		std::string errorString = "Could not load asset: " + uuid.ToString();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return;
	}

	LoadAsset(assetIterator, assetName, outContents, fileSize);
}

// Out:
//	- outContents should be nullptr
//	- fileSize should be 0
void ArchiveAssetLoader::Load(AssetType assetType, std::filesystem::path path, std::string& assetName, char*& outContents, size_t& fileSize) {
	size_t assetTypeIndex = static_cast<size_t>(assetType);

	if (assetTypeIndex >= static_cast<size_t>(AssetType::Count)) {
		outContents = nullptr;
		fileSize = 0;

		std::string errorString = "Invalid Asset Type when trying to load file: " + path.string();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return;
	}

	ArchiveDirectory::AssetInfo* assetInfo = nullptr;
	ArchiveDirectory::AssetTypeIndex& assetTypeSegment = archiveDirectory.assetTypeIndices[static_cast<size_t>(assetType)];
	for (auto& asset : assetTypeSegment.assets) {
		if (asset.second.filename == path) {
			assetInfo = &asset.second;
		}
	}

	if (assetInfo == nullptr) {
		outContents = nullptr;
		fileSize = 0;

		std::string errorString = "Could not load asset: " + path.string();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return;
	}

	LoadAsset(*assetInfo, assetName, outContents, fileSize);
}

bool ArchiveAssetLoader::LoadText(AssetType assetType, Uuid uuid, std::string& assetName, std::string& outContents) {
	char* charPtr;
	size_t fileSize;
	Load(assetType, uuid, assetName, charPtr, fileSize);

	if (charPtr == nullptr || fileSize == 0) {
		std::string errorString = "Could not load file: " + uuid.ToString();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return false;
	}

	outContents = std::string_view(charPtr, fileSize);
	return true;
}

bool ArchiveAssetLoader::LoadText(AssetType assetType, std::filesystem::path path, std::string& assetName, std::string& outContents) {
	char* charPtr;
	size_t fileSize;
	Load(assetType, path, assetName, charPtr, fileSize);

	if (charPtr == nullptr || fileSize == 0) {
		std::string errorString = "Could not load file: " + path.string();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return false;
	}

	outContents = std::string_view(charPtr, fileSize);
	return true;
}

void ArchiveAssetLoader::LoadAsset(const ArchiveDirectory::AssetInfo& assetInfo, std::string& assetName, char*& outContents, size_t& fileSize) {
	if (lastBufferIndex != assetInfo.archiveIndex) {
		lastBufferIndex = assetInfo.archiveIndex;

		const std::string filename = "TestArchive_0.garc";
		const std::filesystem::path path = EngineCore::GetInstance().GetProjectPath() / "archives" / filename;
		const std::string filepathAsStr = path.string();
		const std::vector<char> fileData = Utils::LoadFile(filepathAsStr.c_str());
		lastBuffer = Buffer(fileData.size());
		memcpy(lastBuffer.Get(), fileData.data(), fileData.size());
	}

	BufferView bufferView = lastBuffer.GetBufferView(assetInfo.offset, assetInfo.size);
	outContents = static_cast<char*>(bufferView.Get());
	fileSize = bufferView.GetSize();
	assetName = assetInfo.filename;
}

bool ArchiveAssetLoader::LoadShaderStage(
	const Uuid uuid,
	const GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::ShaderStageCreateInfo& stageCreateInfo,
	std::vector<char>& fileData
) {
	std::string path = GetShaderPath(uuid, shaderStage);
	stageCreateInfo.fileName = path.c_str();

	if (!std::filesystem::exists(path)) {
		std::string errorMsg = path + " shader not found.";
		EngineCore::GetInstance().Print(LogSeverity::Error, errorMsg.c_str());
		return false;
	}

	fileData = Utils::LoadFile(path.c_str());
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
		Grindstone::Logger::PrintError("Incorrect shader stage");
		break;
	}

	std::filesystem::path path = EngineCore::GetInstance().GetAssetPath(uuid.ToString());
	return path.string() + shaderStageExtension + EngineCore::GetInstance().GetGraphicsCore()->GetDefaultShaderExtension();
}
