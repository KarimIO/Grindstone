#include <iostream>
#include <fstream>
#include "EditorManager.hpp"
using namespace Grindstone;

#ifdef _WIN32
#include <Windows.h>
#include <shobjidl.h>

// Request High-Performance GPU for Nvidia and AMD
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

static std::filesystem::path FindFolder() {
	std::filesystem::path outPath;

	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileOpenDialog* pFileOpen = nullptr;

		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr)) {
			DWORD dwOptions;
			if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
				pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
			}

			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr)) {
				IShellItem* pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr)) {
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Display the file name to the user.
					if (SUCCEEDED(hr)) {
						std::wstring tempWStr(pszFilePath);
						outPath = std::filesystem::path(tempWStr);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}

	return outPath;
}

static bool CreateNewProject(const std::filesystem::path& basePath) {
	const std::string folderName = basePath.filename().string();

	// Quit if the base folder doesn't exist and we can't create it.
	if (!std::filesystem::exists(basePath) && !std::filesystem::create_directories(basePath)) {
		return false;
	}

	// Create default folders.
	std::filesystem::create_directories(basePath / "log");
	std::filesystem::create_directories(basePath / "buildSettings");
	std::filesystem::create_directories(basePath / "compiledAssets");
	std::filesystem::create_directories(basePath / "plugins");
	std::filesystem::create_directories(basePath / "userSettings");

	// Create the plugins file (in the future, based on a template).
	{
		std::ofstream outputPluginsFile(basePath / "buildSettings" / "pluginsManifest.txt");
		if (outputPluginsFile.fail()) {
			return false;
		}

		// PluginManifestFileLoader.cpp has an output system but that's a backup.
		// Here is where we should build the default plugins depending on templates
		// when we implement those.
		const std::string defaultProjectPluginName = folderName + ".Main";
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
+ defaultProjectPluginName + "\n";

		outputPluginsFile.write(defaultPlugins.c_str(), defaultPlugins.size());
		outputPluginsFile.close();

		std::filesystem::path defaultProjectPluginPath = basePath / "plugins" / defaultProjectPluginName;
		if (!std::filesystem::exists(defaultProjectPluginPath)) {
			std::filesystem::create_directory(defaultProjectPluginPath);
		}

		std::filesystem::path defaultProjectPluginAssetsPath = defaultProjectPluginPath / "assets";
		if (!std::filesystem::exists(defaultProjectPluginAssetsPath)) {
			std::filesystem::create_directory(defaultProjectPluginAssetsPath);
		}

		std::filesystem::path defaultProjectPluginMetaFilePath = defaultProjectPluginPath / "plugin.meta.json";
		if (!std::filesystem::exists(defaultProjectPluginMetaFilePath)) {
			std::ofstream outputPluginsFile(defaultProjectPluginMetaFilePath);
			if (outputPluginsFile.is_open()) {
				std::string folderNameToUpper = folderName;
				std::transform(folderNameToUpper.begin(), folderNameToUpper.end(), folderNameToUpper.begin(), ::toupper);

				const std::string defaultPlugins = std::string("{\n\
		\"name\": \"" + defaultProjectPluginName + "\",\n\
		\"displayName\" : \"" + folderName + " Main plugin\",\n\
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

	return true;
}

static std::filesystem::path CreateOrOpenExistingProject() {
	std::filesystem::path basePath = FindFolder();

	// Empty path means cancelled, so return.
	if (basePath.empty()) {
		return basePath;
	}

	// Empty folder or nonexistant folder means that there is no existing project, so try create one.
	if (!std::filesystem::exists(basePath) || std::filesystem::is_empty(basePath)) {
		// If we failed to create the project, quit out.
		if (!CreateNewProject(basePath)) {
			return "";
		}
	}
	// Non-empty folder means that the folder already exists so skip creating it and just try open it.

	return basePath;
}
#endif

int main(int argc, char* argv[]) {
		std::filesystem::path projectPath;
		for (int i = 1; i < argc; ++i) {
			if (strcmp(argv[i], "-projectpath") == 0 && argc > i + 1) {
				projectPath = argv[i + 1];
			}
		}

		if (projectPath.empty()) {
	#if _WIN32
			projectPath = CreateOrOpenExistingProject();
			if (projectPath.empty()) {
				std::cerr << "Unable to launch Grindstone Editor - no path set." << std::endl;
				return 1;
			}
	#else
			std::cerr << "Unable to launch Grindstone Editor - no path set." << std::endl;
			return 1;
	#endif
		}

		Editor::Manager editorManager;
		editorManager.SetInstance(&editorManager);
		if (editorManager.Initialize(projectPath)) {
			editorManager.Run();
		}

	return 0;
}
