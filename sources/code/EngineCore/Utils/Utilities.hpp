#pragma once

#include <string>
#include <vector>

namespace Grindstone {
	namespace Utils {
		std::vector<char> LoadFile(const char* inputPath);
		std::string LoadFileText(const char* inputPath);
		void FixStringSlashes(std::string& path);
	}
}
