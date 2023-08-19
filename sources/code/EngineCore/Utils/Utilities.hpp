#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace Grindstone {
	namespace Utils {
		std::vector<char> LoadFile(const char* inputPath);
		std::string LoadFileText(const char* inputPath);
		std::string FixStringSlashesReturn(std::string& path);
		void FixStringSlashes(std::string& path);
		std::filesystem::path FixPathSlashesReturn(std::filesystem::path& path);
		void FixPathSlashes(std::filesystem::path& path);
		std::string ToLower(const std::string& source);
		std::string Trim(const std::string& source);
	}
}
