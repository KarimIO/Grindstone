#include <fstream>
#include <filesystem>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
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
