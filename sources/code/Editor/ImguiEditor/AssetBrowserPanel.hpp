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
				void SetPath(std::filesystem::path path);
				void RefreshAssetsIfNecessary();
				void SortAlphabetically(std::vector<std::filesystem::directory_entry> entries);
				void RefreshAssets();
				void ProcessDirectoryEntryClicks(std::filesystem::directory_entry entry);
				void ClickDirectoryEntry(std::filesystem::directory_entry entry);
				void RenderPath();
				void RenderContextMenuFileTypeSpecificEntries(std::filesystem::directory_entry entry);
				void RenderAssetContextMenu(std::filesystem::directory_entry entry);
				void RenderCurrentDirectoryContextMenu();
				void TryRenameFile();
				void RenderAssetSet(std::vector<std::filesystem::directory_entry> entries);
				void RenderAssets();
				void AfterCreate(std::filesystem::path path);
				ImTextureID GetIcon(const std::filesystem::directory_entry& directoryEntry);
			private:
				std::vector<std::filesystem::directory_entry> sortedDirectories;
				std::vector<std::filesystem::directory_entry> sortedFiles;

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
				std::filesystem::path currentPath;
				std::chrono::time_point<std::chrono::system_clock> lastRefreshedAssetsTime;
			};
		}
	}
}
