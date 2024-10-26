#include <filesystem>
#include <fstream>
#include <iosfwd>
#include <iterator>
#include <string>

#include <Common/Graphics/Core.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>

#include "AssetRegistry.hpp"
#include "EditorManager.hpp"
#include "FileAssetLoader.hpp"

using namespace Grindstone::Assets;

static AssetLoadResult Load(Grindstone::Editor::AssetRegistry::Entry& outEntry, std::string& assetName) {
	assetName = outEntry.name;
	std::filesystem::path path = Grindstone::Editor::Manager::GetEngineCore().GetAssetPath(outEntry.uuid.ToString());

	if (!std::filesystem::exists(path)) {
		return { AssetLoadStatus::FileNotFound, {} };
	}

	std::ifstream file(path, std::ios::binary);
	file.unsetf(std::ios::skipws);

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// TODO: Catch if cannot allocate memory
	Grindstone::Buffer buffer(fileSize);
	file.read(reinterpret_cast<char*>(buffer.Get()), fileSize);

	return std::move(AssetLoadResult{ AssetLoadStatus::Success, std::move(buffer) });
}

AssetLoadResult FileAssetLoader::Load(AssetType assetType, Uuid uuid, std::string& assetName) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(uuid, outEntry)) {
		GPRINT_ERROR_V(LogSource::Editor, "Could not get asset: {}", uuid.ToString());
		return std::move(AssetLoadResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return ::Load(outEntry, assetName);
}

AssetLoadResult FileAssetLoader::Load(AssetType assetType, std::filesystem::path path, std::string& assetName) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(path, outEntry)) {
		std::string errorString = "Could not get asset: " + path.string();
		GPRINT_ERROR(LogSource::Editor, errorString.c_str());
		return std::move(AssetLoadResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return std::move(::Load(outEntry, assetName));
}

static bool LoadText(Grindstone::Editor::AssetRegistry::Entry& outEntry, std::string& assetName, std::string& outContents) {
	assetName = outEntry.name;
	std::filesystem::path path = Grindstone::Editor::Manager::GetEngineCore().GetAssetPath(outEntry.uuid.ToString());

	if (!std::filesystem::exists(path)) {
		return false;
	}

	std::ifstream ifs(path);
	outContents = std::string((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	return true;
}

bool FileAssetLoader::LoadText(AssetType assetType, Uuid uuid, std::string& assetName, std::string& outContents) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(uuid, outEntry)) {
		std::string errorString = "Could not get asset: " + uuid.ToString();
		GPRINT_ERROR(LogSource::Editor, errorString.c_str());
		return false;
	}

	return ::LoadText(outEntry, assetName, outContents);
}

bool FileAssetLoader::LoadText(AssetType assetType, std::filesystem::path path, std::string& assetName, std::string& outContents) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(path, outEntry)) {
		std::string errorString = "Could not get asset: " + path.string();
		GPRINT_ERROR(LogSource::Editor, errorString.c_str());
		return false;
	}

	return ::LoadText(outEntry, assetName, outContents);
}

bool FileAssetLoader::LoadShaderStage(
	Uuid uuid,
	GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	auto& path = GetShaderPath(uuid, shaderStage);
	stageCreateInfo.fileName = path.c_str();

	if (!std::filesystem::exists(path)) {
		std::string errorMsg = path + " shader not found.";
		GPRINT_ERROR(LogSource::Editor, errorMsg.c_str());
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
		GPRINT_ERROR(LogSource::Editor, "Incorrect shader stage");
		break;
	}

	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	std::filesystem::path path = engineCore.GetAssetPath(uuid.ToString());
	return path.string() + shaderStageExtension + engineCore.GetGraphicsCore()->GetDefaultShaderExtension();
}
