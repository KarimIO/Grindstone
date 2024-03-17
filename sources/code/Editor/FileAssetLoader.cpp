#include <fstream>
#include <filesystem>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include "EditorManager.hpp"
#include "AssetRegistry.hpp"
#include <Common/Graphics/Core.hpp>
#include "FileAssetLoader.hpp"
using namespace Grindstone::Assets;

// Out:
//	- outContents should be nullptr
//	- fileSize should be 0
void FileAssetLoader::Load(AssetType assetType, Uuid uuid, char*& outContents, size_t& fileSize) {
	std::filesystem::path path = Editor::Manager::GetEngineCore().GetAssetPath(uuid.ToString());

	if (!std::filesystem::exists(path)) {
		return;
	}

	std::ifstream file(path, std::ios::binary);
	file.unsetf(std::ios::skipws);

	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// TODO: Use allocator
	// TODO: Catch if cannot allocate memory
	outContents = new char[fileSize];
	file.read(outContents, fileSize);
}

// Out:
//	- outContents should be nullptr
//	- fileSize should be 0
void FileAssetLoader::Load(AssetType assetType, std::filesystem::path path, char*& outContents, size_t& fileSize) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(path, outEntry)) {
		std::string errorString = "Could not load file: " + path.string();
		Editor::Manager::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return;
	}

	Load(assetType, outEntry.uuid, outContents, fileSize);
}

bool FileAssetLoader::LoadText(AssetType assetType, Uuid uuid, std::string& outContents) {
	std::filesystem::path path = Editor::Manager::GetEngineCore().GetAssetPath(uuid.ToString());

	if (!std::filesystem::exists(path)) {
		return false;
	}

	std::ifstream ifs(path);
	outContents = std::string((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	return true;
}

bool FileAssetLoader::LoadText(AssetType assetType, std::filesystem::path path, std::string& outContents) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(path, outEntry)) {
		std::string errorString = "Could not load file: " + path.string();
		Editor::Manager::GetInstance().Print(LogSeverity::Error, errorString.c_str());
		return false;
	}

	return LoadText(assetType, outEntry.uuid, outContents);
}

bool FileAssetLoader::LoadShaderStage(
	Uuid uuid,
	GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::ShaderStageCreateInfo& stageCreateInfo,
	std::vector<char>& fileData
) {
	auto& path = GetShaderPath(uuid, shaderStage);
	stageCreateInfo.fileName = path.c_str();

	if (!std::filesystem::exists(path)) {
		std::string errorMsg = path + " shader not found.";
		Editor::Manager::GetInstance().Print(LogSeverity::Error, errorMsg.c_str());
		return false;
	}

	fileData = Utils::LoadFile(path.c_str());
	stageCreateInfo.content = fileData.data();
	stageCreateInfo.size = static_cast<uint32_t>(fileData.size());
	stageCreateInfo.type = shaderStage;

	return true;
}

std::string FileAssetLoader::GetShaderPath(
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
		Editor::Manager::Print(LogSeverity::Error, "Incorrect shader stage");
		break;
	}

	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	std::filesystem::path path = engineCore.GetAssetPath(uuid.ToString());
	return path.string() + shaderStageExtension + engineCore.GetGraphicsCore()->GetDefaultShaderExtension();
}
