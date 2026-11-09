#include <filesystem>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include <Common/Platform/Configuration.hpp>
#include <Common/Platform/Process.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include "Build.hpp"

bool Grindstone::Platform::BuildCmakeTargets(Grindstone::Platform::Configuration configuration, const std::vector<std::string>& cmakeTargets) {
	std::string workingDirectory = Grindstone::EngineCore::GetInstance().GetEngineBasePath().string();
	std::string configurationStr = Grindstone::Platform::ConfigurationToString(configuration);
	std::string platConfig = "Windows" + configurationStr;

	if (!std::filesystem::exists(std::filesystem::path(workingDirectory) / "build" / platConfig)) {
		std::filesystem::create_directories(std::filesystem::path(workingDirectory) / "build" / platConfig);
	}

	std::ostringstream cmdStream;
	cmdStream << "cmake";
	cmdStream << " --build build/" << platConfig;
	cmdStream << " --preset " + platConfig;
	cmdStream << " --parallel";

	if (!cmakeTargets.empty()) {
		cmdStream << " --target";
		for (const std::string& target : cmakeTargets) {
			cmdStream << " \"" << target << "\"";
		}
	}

	std::string commandLine = cmdStream.str();
	return Grindstone::Platform::RunCommand(commandLine, workingDirectory);
}

bool Grindstone::Platform::BuildCmakeSolution(Grindstone::Platform::Configuration configuration) {
	std::string workingDirectory = Grindstone::EngineCore::GetInstance().GetEngineBasePath().string();
	std::string configurationStr = Grindstone::Platform::ConfigurationToString(configuration);
	std::string platConfig = "Windows" + configurationStr;

	if (!std::filesystem::exists(std::filesystem::path(workingDirectory) / "build" / platConfig)) {
		std::filesystem::create_directories(std::filesystem::path(workingDirectory) / "build" / platConfig);
	}

	std::ostringstream cmdStream;
	cmdStream << "cmake";
	cmdStream << " --preset " + platConfig;

	std::string commandLine = cmdStream.str();
	return Grindstone::Platform::RunCommand(commandLine, workingDirectory);
}

bool Grindstone::Platform::BuildDotnetSolution(Grindstone::Platform::Configuration configuration, const std::filesystem::path& path) {
	std::string workingDirectory = (Grindstone::EngineCore::GetInstance().GetEngineBasePath() / "plugins").string();
	std::string configurationStr = Grindstone::Platform::ConfigurationToString(configuration);

	std::ostringstream cmdStream;
	cmdStream << "dotnet build ";
	cmdStream << "--configuration " << configurationStr << " ";
	cmdStream << '\"' << path.string() << '\"';
	std::string commandLine = cmdStream.str();
	return Platform::RunCommand(commandLine, workingDirectory);
}

bool Grindstone::Platform::InstallCmakeBinaries(Grindstone::Platform::Configuration configuration, const std::filesystem::path& targetPath) {
	std::string workingDirectory = Grindstone::EngineCore::GetInstance().GetEngineBasePath() .string();
	std::string configurationStr = Grindstone::Platform::ConfigurationToString(configuration);

	std::ostringstream cmdStream;
	cmdStream << "cmake";
	cmdStream << " --install build/Windows" << configurationStr;
	if (!targetPath.empty()) {
		cmdStream << " --prefix " << targetPath;
	}

	std::string commandLine = cmdStream.str();
	return Grindstone::Platform::RunCommand(commandLine, workingDirectory);
}
