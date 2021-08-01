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
				ImTextureID getIcon(const std::filesystem::directory_entry& directoryEntry);
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
