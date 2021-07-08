#pragma once

#include <chrono>
#include <filesystem>
#include <vector>
#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;
	
	namespace Editor {
		class ComponentInspector;

		namespace ImguiEditor {
			class AssetBrowserPanel {
			public:
				AssetBrowserPanel(EngineCore* engineCore);
				void render();
			private:
				void setPath(std::filesystem::path path);
				void refreshAssetsIfNecessary();
				void sortAlphabetically(std::vector<std::filesystem::directory_entry> entries);
				void refreshAssets();
				void clickDirectoryEntry(std::filesystem::directory_entry entry);
				void renderPath();
				void renderAssetContextMenu(std::filesystem::directory_entry entry);
				void renderCurrentDirectoryContextMenu();
				void tryRenameFile();
				void renderAssetSet(std::vector<std::filesystem::directory_entry> entries);
				void renderAssets();
			private:
				std::vector<std::filesystem::directory_entry> sortedDirectories;
				std::vector<std::filesystem::directory_entry> sortedFiles;

				bool isShowingPanel = true;
				EngineCore* engineCore = nullptr;
				std::filesystem::path pathToRename;
				std::string pathRenameNewName;
				std::filesystem::path currentPath;
				const float padding = 8.0f;
				const float thumbnailSize = 80.0f;
				std::chrono::time_point<std::chrono::system_clock> lastRefreshedAssetsTime;
			};
		}
	}
}
