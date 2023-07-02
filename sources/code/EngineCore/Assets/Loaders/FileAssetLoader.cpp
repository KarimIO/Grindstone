#include <fstream>
#include <filesystem>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Logger.hpp>
#include <Common/Graphics/Core.hpp>
#include "FileAssetLoader.hpp"
using namespace Grindstone::Assets;

// Out:
//	- outContents should be nullptr
//	- fileSize should be 0
void FileAssetLoader::Load(Uuid uuid, char*& outContents, size_t& fileSize) {
	std::filesystem::path path = EngineCore::GetInstance().GetAssetPath(uuid.ToString());
	Load(path, outContents, fileSize);
}

// Out:
//	- outContents should be nullptr
//	- fileSize should be 0
void FileAssetLoader::Load(std::filesystem::path path, char*& outContents, size_t& fileSize) {
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

bool FileAssetLoader::LoadText(Uuid uuid, std::string& outContents) {
	std::filesystem::path path = EngineCore::GetInstance().GetAssetPath(uuid.ToString());

	if (!std::filesystem::exists(path)) {
		return false;
	}

	std::ifstream ifs(path);
	outContents = std::string((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	return true;
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
		EngineCore::GetInstance().Print(LogSeverity::Error, errorMsg.c_str());
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
		Grindstone::Logger::PrintError("Incorrect shader stage");
		break;
	}

	std::filesystem::path path = EngineCore::GetInstance().GetAssetPath(uuid.ToString());
	return path.string() + shaderStageExtension + EngineCore::GetInstance().GetGraphicsCore()->GetDefaultShaderExtension();
}
