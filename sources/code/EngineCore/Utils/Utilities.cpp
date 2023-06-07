#include <algorithm>
#include <filesystem>
#include <fstream>

#include "Utilities.hpp"

using namespace Grindstone;
std::vector<char> Utils::LoadFile(const char* inputPath) {
	if (!std::filesystem::exists(inputPath)) {
		return {};
	}

	std::ifstream file(inputPath, std::ios::binary);
	file.unsetf(std::ios::skipws);

	std::streampos fileSize;

	file.seekg(0, std::ios::end);
	fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> fileData;
	fileData.reserve(fileSize);

	fileData.insert(fileData.begin(),
		std::istream_iterator<char>(file),
		std::istream_iterator<char>()
	);

	file.close();

	return fileData;
}

std::string Utils::LoadFileText(const char* inputPath) {
	std::ifstream ifs(inputPath);
	std::string content((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));

	return content;
}

std::string Utils::FixStringSlashesReturn(std::string path) {
	std::string swappedPath = path;
	std::replace(swappedPath.begin(), swappedPath.end(), '\\', '/');
	return swappedPath;
}

void Utils::FixStringSlashes(std::string& path) {
	std::replace(path.begin(), path.end(), '\\', '/');
}
