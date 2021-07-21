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
			class ImguiEditor;
			class AssetBrowserPanel {
			public:
				AssetBrowserPanel(EngineCore* engineCore, ImguiEditor* editor);
				void render();
			private:
				void setPath(std::filesystem::path path);
				void refreshAssetsIfNecessary();
				void sortAlphabetically(std::vector<std::filesystem::directory_entry> entries);
				void refreshAssets();
				void processDirectoryEntryClicks(std::filesystem::directory_entry entry);
				void clickDirectoryEntry(std::filesystem::directory_entry entry);
				void renderPath();
				void renderContextMenuConvertButton(std::filesystem::directory_entry entry);
				void renderAssetContextMenu(std::filesystem::directory_entry entry);
				void renderCurrentDirectoryContextMenu();
				void tryRenameFile();
				void renderAssetSet(std::vector<std::filesystem::directory_entry> entries);
				void renderAssets();
				void afterCreate(std::filesystem::path path);
			private:
				std::vector<std::filesystem::directory_entry> sortedDirectories;
				std::vector<std::filesystem::directory_entry> sortedFiles;

				bool isShowingPanel = true;
				EngineCore* engineCore = nullptr;
				ImguiEditor* editor = nullptr;
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
