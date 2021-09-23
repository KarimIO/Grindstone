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
			class AssetBrowserPanel {
			public:
				AssetBrowserPanel(EngineCore* engineCore, ImguiEditor* editor);
				void Render();
			private:
				struct Directory {
					Directory* parentDirectory = nullptr;
					std::filesystem::directory_entry path;
					std::vector<Directory*> subdirectories;
					std::vector<std::filesystem::directory_entry> files;

					Directory() = default;
					Directory(std::filesystem::directory_entry path, Directory* parentDirectory) :
						path(path),
						parentDirectory(parentDirectory) {}
				};

				void CreateInitialFileStructure(Directory& directory, std::filesystem::directory_iterator);
				void SetCurrentAssetDirectory(Directory& directory);
				void ProcessDirectoryEntryClicks(std::filesystem::directory_entry entry, Directory* directory = nullptr);
				void RenderTopBar();
				void RenderPathPart(Directory* path);
				void RenderPath();
				void RenderContextMenuFileTypeSpecificEntries(std::filesystem::directory_entry entry);
				void RenderAssetContextMenu(std::filesystem::directory_entry entry);
				void RenderCurrentDirectoryContextMenu();
				void TryRenameFile();
				void RenderFolders();
				void RenderFiles();
				void RenderAssets();
				void RenderSidebar();
				void RenderSidebarSubdirectory(Directory& directory);
				void AfterCreate(std::filesystem::path path);
				ImTextureID GetIcon(const std::filesystem::directory_entry& directoryEntry);
			private:
				Directory rootDirectory;
				Directory& currentDirectory;

				struct Icons {
					GraphicsAPI::Texture* folder;
					GraphicsAPI::Texture* file;
					GraphicsAPI::Texture* font;
					GraphicsAPI::Texture* image;
					GraphicsAPI::Texture* material;
					GraphicsAPI::Texture* model;
					GraphicsAPI::Texture* scene;
					GraphicsAPI::Texture* shader;
					GraphicsAPI::Texture* sound;
					GraphicsAPI::Texture* text;
					GraphicsAPI::Texture* video;
				} iconTextures;

				struct IconsIds {
					ImTextureID folder;
					ImTextureID file;
					ImTextureID font;
					ImTextureID image;
					ImTextureID material;
					ImTextureID model;
					ImTextureID scene;
					ImTextureID shader;
					ImTextureID sound;
					ImTextureID text;
					ImTextureID video;
				} iconIds;

				bool isShowingPanel = true;
				EngineCore* engineCore = nullptr;
				ImguiEditor* editor = nullptr;
				std::filesystem::path pathToRename;
				std::string pathRenameNewName;
				std::string searchText;
				std::chrono::time_point<std::chrono::system_clock> lastRefreshedAssetsTime;
			};
		}
	}
}
