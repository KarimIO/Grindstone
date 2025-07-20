#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <Common/Buffer.hpp>

namespace Grindstone::Utils {
	Grindstone::Buffer LoadFile(const char* inputPath);
	std::string LoadFileText(const char* inputPath);
	std::string FixStringSlashesReturn(const std::string& path);
	void FixStringSlashes(std::string& path);
	std::filesystem::path FixPathSlashesReturn(const std::filesystem::path& path);
	void FixPathSlashes(std::filesystem::path& path);
	std::string ToLower(const std::string& source);
	std::string TrimLeft(const std::string& source);
	std::string TrimRight(const std::string& source);
	std::string Trim(const std::string& source);
}
