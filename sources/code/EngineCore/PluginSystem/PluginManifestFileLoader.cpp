#include <EngineCore/Logger.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include "PluginManifestFileLoader.hpp"

bool Grindstone::Plugins::LoadPluginManifestFile(std::vector<Grindstone::Plugins::ManifestData>& manifestData) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	std::filesystem::path pluginListFile = engineCore.GetProjectPath() / "buildSettings/pluginsManifest.txt";

	std::string pluginListFileString = pluginListFile.string();
	if (!std::filesystem::exists(pluginListFile)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Expected plugin list file at {} but couldn't find one.", pluginListFileString.c_str());
		return false;
	}

	std::string fileContents = Grindstone::Utils::LoadFileText(pluginListFileString.c_str());

	size_t start = 0, end;
	std::string pluginName;
	while (true) {
		end = fileContents.find("\n", start);
		if (end == std::string::npos) {
			pluginName = Grindstone::Utils::Trim(fileContents.substr(start));
			if (!pluginName.empty()) {
				manifestData.emplace_back(
					ManifestData{
						.pluginName = pluginName.c_str(),
						.semanticVersioning = ">0.0.1",
						.path = ""
					}
				);
			}

			break;
		}

		pluginName = Grindstone::Utils::Trim(fileContents.substr(start, end - start));
		if (!pluginName.empty()) {
			manifestData.emplace_back(
				ManifestData{
					.pluginName = pluginName.c_str(),
					.semanticVersioning = ">0.0.1",
					.path = ""
				}
			);
		}
		start = end + 1;
	}

	return true;
}
