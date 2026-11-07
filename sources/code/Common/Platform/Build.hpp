#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <Common/Platform/Configuration.hpp>

namespace Grindstone::Platform {
	bool BuildCmakeSolution(Grindstone::Platform::Configuration configuration, const std::vector<std::string>& cmakeTargets);
	bool BuildDotnetSolution(Grindstone::Platform::Configuration configuration, const std::filesystem::path& path);
	bool InstallCmakeBinaries(Grindstone::Platform::Configuration configuration, const std::filesystem::path& targetPath);
	}
