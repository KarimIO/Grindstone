#include <filesystem>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <Common/Platform/Configuration.hpp>
#include <Common/Platform/Process.hpp>
#include <EngineCore/EngineCore.hpp>

#include "Build.hpp"

bool Grindstone::Platform::BuildCmakeSolution(Grindstone::Platform::Configuration configuration, const std::vector<std::string>& cmakeTargets) {
	std::string workingDirectory = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path().string();
	std::string configurationStr = Grindstone::Platform::ConfigurationToString(configuration);

	std::ostringstream cmdStream;
	cmdStream << "cmake";
	cmdStream << " --build build/windows-" << configurationStr;
	cmdStream << " --preset windows-" + configurationStr;
	cmdStream << " --parallel";

	if (cmakeTargets.empty()) {
		cmdStream << " --target";
		for (const std::string& target : cmakeTargets) {
			cmdStream << " \"" << target << "\"";
		}
	}

	std::string commandLine = cmdStream.str();
	return Grindstone::Platform::RunCommand(commandLine, workingDirectory);
}

bool Grindstone::Platform::BuildDotnetSolution(Grindstone::Platform::Configuration configuration, const std::filesystem::path& path) {
	std::string workingDirectory = (Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path() / "plugins").string();
	std::string configurationStr = Grindstone::Platform::ConfigurationToString(configuration);

	std::ostringstream cmdStream;
	cmdStream << "dotnet build ";
	cmdStream << "--configuration " << configurationStr << " ";
	cmdStream << '\"' << path.string() << '\"';
	std::string commandLine = cmdStream.str();
	return Platform::RunCommand(commandLine, workingDirectory);
}

bool Grindstone::Platform::InstallCmakeBinaries(Grindstone::Platform::Configuration configuration, const std::filesystem::path& targetPath) {
	std::string workingDirectory = Grindstone::EngineCore::GetInstance().GetEngineBinaryPath().parent_path().string();
	std::string configurationStr = Grindstone::Platform::ConfigurationToString(configuration);

	std::ostringstream cmdStream;
	cmdStream << "cmake";
	cmdStream << " --install build/windows-" << configurationStr;
	cmdStream << " --prefix " << targetPath;

	std::string commandLine = cmdStream.str();
	return Grindstone::Platform::RunCommand(commandLine, workingDirectory);
}
