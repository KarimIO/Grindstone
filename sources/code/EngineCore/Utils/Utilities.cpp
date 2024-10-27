#include <algorithm>
#include <filesystem>
#include <fstream>

#include <Common/Buffer.hpp>
#include <EngineCore/Logger.hpp>

#include "Utilities.hpp"

using namespace Grindstone;

Grindstone::Buffer Utils::LoadFile(const char* inputPath) {
	if (!std::filesystem::exists(inputPath)) {
		return {};
	}

	std::ifstream file(inputPath, std::ios::binary);
	file.unsetf(std::ios::skipws);

	file.seekg(0, std::ios::end);
	const std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	Grindstone::Buffer buffer(fileSize);
	file.read(reinterpret_cast<char*>(buffer.Get()), fileSize);

	file.close();

	return buffer;
}

std::string Utils::LoadFileText(const char* inputPath) {
	if (!std::filesystem::exists(inputPath)) {
		return "";
	}

	std::ifstream ifs(inputPath, std::ios::in);
	std::string outStr;
	std::getline(ifs, outStr, '\0');

	return outStr;
}

std::string Utils::FixStringSlashesReturn(std::string& path) {
	std::string swappedPath = path;
	std::replace(swappedPath.begin(), swappedPath.end(), '\\', '/');
	return swappedPath;
}

void Utils::FixStringSlashes(std::string& path) {
	std::replace(path.begin(), path.end(), '\\', '/');
}

std::filesystem::path Utils::FixPathSlashesReturn(std::filesystem::path& path) {
	std::string swappedPath = path.string();
	std::replace(swappedPath.begin(), swappedPath.end(), '\\', '/');
	return swappedPath;
}

void Utils::FixPathSlashes(std::filesystem::path& path) {
	std::string swappedPath = path.string();
	std::replace(swappedPath.begin(), swappedPath.end(), '\\', '/');
	path = swappedPath;
}

std::string Utils::ToLower(const std::string& source) {
	std::string outString(source.length(), 0);
	std::transform(source.begin(), source.end(), outString.begin(), ::tolower);
	return outString;
}

const char* whitespace = " \t\f\v\r\n";

std::string& Utils::TrimLeft(std::string& string) {
	string.erase(0, string.find_first_not_of(whitespace));
	return string;
}

std::string& Utils::TrimRight(std::string& string) {
	string.erase(string.find_last_not_of(whitespace) + 1);
	return string;
}

std::string& Utils::Trim(std::string& string) {
	return Utils::TrimRight(Utils::TrimLeft(string));
}
