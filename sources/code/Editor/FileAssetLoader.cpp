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

static AssetLoadBinaryResult LoadBinary(Grindstone::Editor::AssetRegistry::Entry& entry) {
	std::filesystem::path path = Grindstone::Editor::Manager::GetEngineCore().GetAssetPath(entry.uuid.ToString());

	if (!std::filesystem::exists(path)) {
		return { AssetLoadStatus::FileNotFound, {} };
	}

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// TODO: Catch if cannot allocate memory
	Grindstone::Buffer buffer(fileSize);
	file.read(reinterpret_cast<char*>(buffer.Get()), fileSize);

	return std::move(AssetLoadBinaryResult{ AssetLoadStatus::Success, entry.displayName, std::move(buffer) });
}

static AssetLoadTextResult LoadText(Grindstone::Editor::AssetRegistry::Entry& entry) {
	std::filesystem::path path = Grindstone::Editor::Manager::GetEngineCore().GetAssetPath(entry.uuid.ToString());

	if (!std::filesystem::exists(path)) {
		return { AssetLoadStatus::FileNotFound, {} };
	}

	std::ifstream ifs(path, std::ios::in);

	std::string contents;
	std::getline(ifs, contents, '\0');

	return std::move(AssetLoadTextResult{ AssetLoadStatus::Success, entry.displayName, std::move(contents) });
}

AssetLoadBinaryResult FileAssetLoader::LoadBinaryByPath(AssetType assetType, const std::filesystem::path& path) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(path, outEntry)) {
		std::string errorString = "Could not get asset: " + path.string();
		GPRINT_ERROR(LogSource::Editor, errorString.c_str());
		return std::move(AssetLoadBinaryResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return std::move(::LoadBinary(outEntry));
}

AssetLoadBinaryResult FileAssetLoader::LoadBinaryByAddress(AssetType assetType, std::string_view address) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	std::string addressString = std::string(address);
	if (!assetRegistry.TryGetAssetData(addressString, outEntry)) {
		std::string errorString = "Could not get asset: " + std::string(address);
		GPRINT_ERROR(LogSource::Editor, errorString.c_str());
		return std::move(AssetLoadBinaryResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return std::move(::LoadBinary(outEntry));
}

AssetLoadBinaryResult FileAssetLoader::LoadBinaryByUuid(AssetType assetType, Uuid uuid) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(uuid, outEntry)) {
		GPRINT_ERROR_V(LogSource::Editor, "Could not get asset: {}", uuid.ToString());
		return std::move(AssetLoadBinaryResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return ::LoadBinary(outEntry);
}

AssetLoadTextResult FileAssetLoader::LoadTextByPath(AssetType assetType, const std::filesystem::path& path) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(path, outEntry)) {
		std::string errorString = "Could not get asset: " + path.string();
		GPRINT_ERROR(LogSource::Editor, errorString.c_str());
		return std::move(AssetLoadTextResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return std::move(::LoadText(outEntry));
}

AssetLoadTextResult FileAssetLoader::LoadTextByAddress(AssetType assetType, std::string_view address) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	std::string addressString = std::string(address);
	if (!assetRegistry.TryGetAssetData(addressString, outEntry)) {
		std::string errorString = "Could not get asset: " + std::string(address);
		GPRINT_ERROR(LogSource::Editor, errorString.c_str());
		return std::move(AssetLoadTextResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return std::move(::LoadText(outEntry));
}

AssetLoadTextResult FileAssetLoader::LoadTextByUuid(AssetType assetType, Uuid uuid) {
	Editor::AssetRegistry& assetRegistry = Editor::Manager::GetInstance().GetAssetRegistry();

	Editor::AssetRegistry::Entry outEntry;
	if (!assetRegistry.TryGetAssetData(uuid, outEntry)) {
		GPRINT_ERROR_V(LogSource::Editor, "Could not get asset: {}", uuid.ToString());
		return std::move(AssetLoadTextResult{ AssetLoadStatus::AssetNotInRegistry, {} });
	}

	return ::LoadText(outEntry);
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

	const char* shaderExt = Grindstone::Editor::Manager::GetEngineCore().GetGraphicsCore()->GetDefaultShaderExtension();
	return path.string() + shaderStageExtension + shaderExt;
}

bool Grindstone::Assets::FileAssetLoader::LoadShaderStageByPath(
	const std::filesystem::path& path,
	GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	std::filesystem::path stagePath = ::GetShaderPath(path, shaderStage);
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

bool Grindstone::Assets::FileAssetLoader::LoadShaderStageByAddress(
	std::string_view address,
	GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	auto& registry = Editor::Manager::GetInstance().GetAssetRegistry();

	Grindstone::Editor::AssetRegistry::Entry outEntry;
	std::string addressStr = std::string(address);
	if (!registry.TryGetAssetData(addressStr, outEntry)) {
		return false;
	}

	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	std::filesystem::path basePath = engineCore.GetAssetPath(outEntry.uuid.ToString());
	auto stagePath = ::GetShaderPath(basePath, shaderStage);
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

bool FileAssetLoader::LoadShaderStageByUuid(
	Uuid uuid,
	GraphicsAPI::ShaderStage shaderStage,
	GraphicsAPI::GraphicsPipeline::CreateInfo::ShaderStageData& stageCreateInfo,
	std::vector<char>& fileData
) {
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
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
