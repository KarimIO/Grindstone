#pragma once

#include <chrono>
#include <filesystem>
#include <vector>
#include <entt/entt.hpp>

namespace Grindstone {
	class EngineCore;
}

namespace Grindstone::GraphicsAPI {
	class Texture;
}
	
namespace Grindstone::Editor {
	class ComponentInspector;
}

namespace Grindstone::Editor::ImguiEditor {
	class ImguiEditor;
	class ImguiRenderer;
	class AssetBrowserPanel {
	public:
		AssetBrowserPanel(ImguiRenderer* imguiRenderer, EngineCore* engineCore, ImguiEditor* editor);
		void Render();
	private:

		struct AssetBrowserItem {
			struct Subasset {
				Grindstone::Uuid uuid;
				std::string name;
				AssetType assetType;
			};

			AssetType assetType;
			std::filesystem::path filepath;
			std::filesystem::path filename;
			Grindstone::Uuid defaultUuid;
			bool isSubassetListOpen;
			std::vector<Subasset> subassets;
		};

		void AddFilePath(const std::filesystem::directory_entry& file);

		void SetCurrentAssetDirectory(const std::filesystem::path& path);
		void SetFilesFromCurrentDirectory();
		void ProcessDirectoryEntryClicks(bool isFolder, const std::filesystem::path& path);
		void RenderTopBar();
		void RenderPathPart(const std::filesystem::path& path);
		void RenderPath();
		void RenderContextMenuFileTypeSpecificEntries(const std::filesystem::path& path);
		void RenderAssetContextMenu(bool isFolder, const std::filesystem::path& path, size_t index);
		void RenderCurrentDirectoryContextMenu();
		void RenderAssetTemplates(const std::filesystem::path& path);
		size_t SortFile(bool isFolder, size_t indexToSort);
		void TryRenameFile();
		void RenderFolders();
		void RenderFiles();
		void RenderAssets(float height);
		void RenderSidebar(float height);
		void RenderSidebarSubdirectory(const std::filesystem::directory_entry& entry);
		void RenderFile(size_t fileIndex);
		void FilterSearch();
		void AfterCreate(const std::filesystem::path& path);
		ImTextureID GetIcon(const AssetType assetType) const;
	private:
		std::filesystem::directory_entry currentDirectory;
		// TODO: Handle when the current mounting point
		const Grindstone::Editor::FileManager::MountPoint* currentMountingPoint = nullptr;

		struct IconsIds {
			ImTextureID folderIcon;
			ImTextureID fileIcons[static_cast<uint16_t>(AssetType::Count)];
		} iconIds;

		bool isShowingPanel = true;
		EngineCore* engineCore = nullptr;
		ImguiEditor* editor = nullptr;
		bool isRenamingFolder;
		size_t indexToRename;
		std::string pathRenameNewName;
		std::string searchText;
		std::string searchTextLower;
		std::vector<std::filesystem::path> folders;
		std::vector<AssetBrowserItem> files;
		std::chrono::time_point<std::chrono::system_clock> lastRefreshedAssetsTime;
	};
}
