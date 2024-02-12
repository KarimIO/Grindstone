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
void ArchiveAssetLoader::Load(AssetType assetType, Uuid uuid, char*& outContents, size_t& fileSize) {
	size_t assetTypeIndex = static_cast<size_t>(assetType);

	if (assetTypeIndex >= static_cast<size_t>(AssetType::Count)) {
		outContents = nullptr;
		fileSize = 0;

		std::string errorString = "Invalid Asset Type when trying to load file: " + uuid.ToString();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return;
	}

	ArchiveDirectory::AssetTypeIndex& assetTypeSegment = archiveDirectory.assetTypeIndices[static_cast<size_t>(assetType)];
	auto& assetIterator = assetTypeSegment.assets.find(uuid);

	if (assetIterator == assetTypeSegment.assets.end()) {
		outContents = nullptr;
		fileSize = 0;

		std::string errorString = "Could not load asset: " + uuid.ToString();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return;
	}

	LoadAsset(assetIterator->second, outContents, fileSize);
}

// Out:
//	- outContents should be nullptr
//	- fileSize should be 0
void ArchiveAssetLoader::Load(AssetType assetType, std::filesystem::path path, char*& outContents, size_t& fileSize) {
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

	LoadAsset(*assetInfo, outContents, fileSize);
}

bool ArchiveAssetLoader::LoadText(AssetType assetType, Uuid uuid, std::string& outContents) {
	char* charPtr;
	size_t fileSize;
	Load(assetType, uuid, charPtr, fileSize);

	if (charPtr == nullptr || fileSize == 0) {
		std::string errorString = "Could not load file: " + uuid.ToString();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return false;
	}

	outContents = std::string_view(charPtr, fileSize);
	return true;
}

bool ArchiveAssetLoader::LoadText(AssetType assetType, std::filesystem::path path, std::string& outContents) {
	char* charPtr;
	size_t fileSize;
	Load(assetType, path, charPtr, fileSize);

	if (charPtr == nullptr || fileSize == 0) {
		std::string errorString = "Could not load file: " + path.string();
		EngineCore::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return false;
	}

	outContents = std::string_view(charPtr, fileSize);
	return true;
}

void ArchiveAssetLoader::LoadAsset(ArchiveDirectory::AssetInfo& assetInfo, char*& outContents, size_t& fileSize) {
	if (lastBufferIndex != assetInfo.archiveIndex) {
		lastBufferIndex = assetInfo.archiveIndex;

		std::string filename = "TestArchive_0.garc";
		std::filesystem::path path = EngineCore::GetInstance().GetProjectPath() / "archives" / filename;
		std::string filepathAsStr = path.string();
		std::vector<char> fileData = Utils::LoadFile(filepathAsStr.c_str());
		lastBuffer = Buffer(fileData.data(), fileData.size());
	}

	BufferView bufferView = lastBuffer.GetBufferView(assetInfo.offset, assetInfo.size);
	outContents = static_cast<char*>(bufferView.Get());
	fileSize = bufferView.GetSize();
}

bool ArchiveAssetLoader::LoadShaderStage(
	Uuid uuid,
	GraphicsAPI::ShaderStage shaderStage,
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
