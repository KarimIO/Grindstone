#include <EngineCore/Logger.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include "PluginManifestFileLoader.hpp"

static void CreateDefaultPluginManifestFile(
	std::string projectFolderName,
	const std::filesystem::path& manifestPath,
	const std::filesystem::path& pluginsFolderPath
) {
	std::string folderNameToUpper = projectFolderName + ".Main";
	const std::string defaultPlugins = std::string("Grindstone.Editor.Assets\n\
Grindstone.Editor.AudioImporter\n\
Grindstone.Editor.MaterialImporter\n\
Grindstone.Editor.ModelImporter\n\
Grindstone.Editor.PipelineSetImporter\n\
Grindstone.Editor.TextureImporter\n\
Grindstone.Physics.Bullet\n\
Grindstone.Renderables.3D\n\
Grindstone.Renderer.Deferred\n\
Grindstone.RHI.Vulkan\n\
Grindstone.Script.CSharp\n")
+ folderNameToUpper + "\n";

	std::filesystem::create_directories(manifestPath.parent_path());

	std::ofstream outputPlugins(manifestPath);
	outputPlugins.write(defaultPlugins.c_str(), defaultPlugins.size());
	outputPlugins.close();

	std::string defaultProjectPluginName = "";
	std::filesystem::path defaultProjectPluginPath = pluginsFolderPath.parent_path() / "plugins" / defaultProjectPluginName;
	std::filesystem::path defaultProjectPluginMetaFilePath = defaultProjectPluginPath / "plugin.meta.json";
	if (!std::filesystem::exists(defaultProjectPluginMetaFilePath)) {
		std::ofstream outputPluginsFile(defaultProjectPluginMetaFilePath);
		if (outputPluginsFile.is_open()) {
			std::transform(folderNameToUpper.begin(), folderNameToUpper.end(), folderNameToUpper.begin(), ::toupper);

			const std::string defaultPlugins = std::string("{\n\
	\"name\": \"" + defaultProjectPluginName + "\",\n\
	\"displayName\" : \"" + projectFolderName + " Main plugin\",\n\
	\"version\" : \"0.0.1\",\n\
	\"description\" : \"The main plugin of the your project.\",\n\
	\"author\" : \"\",\n\
	\"assetDirectories\" : [\n\
		{\n\
			\"path\": \"assets\",\n\
				\"mountPoint\" : \"" + folderNameToUpper + ".MAIN\", \n\
				\"loadStage\" : \"EditorAssetImportEarly\"\n\
		}\n\
	], \n\
	\"dependencies\": [],\n\
	\"binaries\" : [],\n\
	\"cmake\": \"\"\n\
}\n\
");

			outputPluginsFile.write(defaultPlugins.c_str(), defaultPlugins.size());
			outputPluginsFile.close();
		}
	}
}

bool Grindstone::Plugins::LoadPluginManifestFile(std::vector<Grindstone::Plugins::ManifestData>& manifestData) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	std::filesystem::path projectDir = engineCore.GetProjectPath();
	std::filesystem::path pluginListFile = projectDir / "buildSettings" / "pluginsManifest.txt";

	std::string pluginListFileString = pluginListFile.string();
	if (!std::filesystem::exists(pluginListFile)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Expected plugin list file at {} but couldn't find one. Making a new one.", pluginListFileString.c_str());

		std::filesystem::path pluginFolderDir = projectDir / "plugins";
		std::string projectName = projectDir.filename().string();
		CreateDefaultPluginManifestFile(projectDir.filename().string(), pluginListFile, pluginFolderDir);
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
