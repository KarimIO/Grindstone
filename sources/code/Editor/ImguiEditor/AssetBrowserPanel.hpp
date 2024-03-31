#pragma once

#include <chrono>
#include <filesystem>
#include <vector>
#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;

	namespace GraphicsAPI {
		class Texture;
	}
	
	namespace Editor {
		class ComponentInspector;

		namespace ImguiEditor {
			class ImguiEditor;
			class ImguiRenderer;
			class AssetBrowserPanel {
			public:
				AssetBrowserPanel(ImguiRenderer* imguiRenderer, EngineCore* engineCore, ImguiEditor* editor);
				void Render();
			private:
				void CreateInitialFileStructure(Directory& directory, std::filesystem::directory_iterator);
				void SetCurrentAssetDirectory(Directory& directory);
				void ProcessDirectoryEntryClicks(std::filesystem::directory_entry entry, Directory* directory = nullptr);
				void RenderTopBar();
				void RenderPathPart(Directory* path);
				void RenderPath();
				void RenderContextMenuFileTypeSpecificEntries(std::filesystem::directory_entry entry);
				void RenderAssetContextMenu(std::filesystem::directory_entry entry);
				void RenderCurrentDirectoryContextMenu();
				void RenderAssetTemplates(const std::filesystem::path& path);
				void TryRenameFile();
				void RenderFolders();
				void RenderFiles();
				void RenderAssets(float height);
				void RenderSidebar(float height);
				void RenderSidebarSubdirectory(Directory& directory);
				void RenderFile(File* file);
				void FilterSearch(Directory& dir);
				void FilterSearch();
				void AfterCreate(std::filesystem::path path);
				ImTextureID GetIcon(const AssetType assetType) const;
			private:
				Directory& rootDirectory;
				Directory* currentDirectory;

				struct IconsIds {
					ImTextureID folderIcon;
					ImTextureID fileIcons[static_cast<uint16_t>(AssetType::Count)];
				} iconIds;

				bool isShowingPanel = true;
				EngineCore* engineCore = nullptr;
				ImguiEditor* editor = nullptr;
				std::filesystem::path pathToRename;
				std::string pathRenameNewName;
				std::string searchText;
				std::string searchTextLower;
				std::set<Uuid> expandedAssetUuidsWithSubassets;
				std::vector<File*> searchedFiles;
				std::chrono::time_point<std::chrono::system_clock> lastRefreshedAssetsTime;
			};
		}
	}
}
