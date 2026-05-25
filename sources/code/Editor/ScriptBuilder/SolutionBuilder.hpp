#pragma once

#include <vector>
#include <string>

#include "CSharpProjectMetaData.hpp"

namespace Grindstone::Editor::ScriptBuilder {
	void CreateSolutionFile(const std::filesystem::path& slnPath, const std::vector<std::filesystem::path>& projects);
}
