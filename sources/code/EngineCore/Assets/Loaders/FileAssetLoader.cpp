#include <fstream>
#include <filesystem>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/EngineCore.hpp>
#include "FileAssetLoader.hpp"
using namespace Grindstone::Assets;

void FileAssetLoader::Load(Uuid uuid, std::vector<char>& outContents) {
	std::filesystem::path path = EngineCore::GetInstance().GetAssetPath(uuid.ToString());

	std::ifstream file(path, std::ios::binary);
	file.unsetf(std::ios::skipws);

	std::streampos fileSize;

	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	outContents.reserve(fileSize);

	outContents.insert(outContents.begin(),
		std::istream_iterator<char>(file),
		std::istream_iterator<char>()
	);
}
