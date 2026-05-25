#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "CSharpProjectMetaData.hpp"

namespace Grindstone::Editor::ScriptBuilder {
	void CreateProjectFile(const std::filesystem::path& projectPath, const std::vector<std::filesystem::path>& references);
}
