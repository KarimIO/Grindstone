#include <algorithm>
#include <fstream>
#include <string>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <entt/entt.hpp>
#include "ComponentInspector.hpp"
#include "Editor/Converters/ShaderImporter.hpp"
#include "Editor/Converters/ModelConverter.hpp"
#include "Editor/Converters/TextureConverter.hpp"
#include "Editor/EditorManager.hpp"
#include "AssetBrowserPanel.hpp"
#include "Common/Window/WindowManager.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Assets/Textures/TextureManager.hpp"
#include "Plugins/GraphicsOpenGL/GLTexture.hpp"
#include "ImguiEditor.hpp"

const std::filesystem::path ASSET_FOLDER_PATH = "..\\assets";
const double REFRESH_INTERVAL = 1.0;
const bool RIGHT_MOUSE_BUTTON = 1;
const float PADDING = 8.0f;
const float ENTRY_SIZE = 80.0f;
const float THUMBNAIL_SIZE = 64.0f;
const float THUMBNAIL_PADDING = 4.0f;
const float THUMBNAIL_SPACING = (ENTRY_SIZE - THUMBNAIL_SIZE - THUMBNAIL_PADDING) / 2.0f;

std::filesystem::path GetNewDefaultPath(std::filesystem::path basePath, std::string fileName, std::string extension) {
	std::filesystem::path finalPath = basePath / (fileName + extension);
	if (!std::filesystem::exists(finalPath)) {
		return finalPath;
	}

	size_t i = 2;
	while (true) {
		finalPath = basePath / (fileName + " (" + std::to_string(i++) + ")" + extension);
		if (!std::filesystem::exists(finalPath)) {
			return finalPath;
		}
	}
}

std::filesystem::path CreateDefaultMaterial(std::filesystem::path& currentPath) {
	std::filesystem::path path = GetNewDefaultPath(currentPath, "New Material", ".gmat");
	std::ofstream output(path);
	output << "{\n\t\"name\": \"New Material\"\n\t\"shader\": \"\"\n}";
	output.close();

	return path;
}

ImTextureID getIdFromTexture(GraphicsAPI::Texture* texture) {
	GraphicsAPI::GLTexture* glTex = (GraphicsAPI::GLTexture*)texture;
	return (ImTextureID)glTex->getTexture();
}

void TextCenter(std::string text) {
	float font_size = ImGui::GetFontSize() * text.size() / 2;
	ImGui::SameLine(
		ImGui::GetWindowSize().x / 2 -
		font_size + (font_size / 2)
	);

	ImGui::Text(text.c_str());
}

void PrepareIcon(Grindstone::TextureManager* textureManager, const char* path, GraphicsAPI::Texture*& texture, ImTextureID& id) {
	auto& textureAsset = textureManager->LoadTexture(path);
	texture = textureAsset.texture;
	id = getIdFromTexture(texture);
}

#define PREPARE_ICON(type) PrepareIcon(textureManager, "../engineassets/editor/assetIcons/" #type ".dds", iconTextures.type, iconIds.type)

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			AssetBrowserPanel::AssetBrowserPanel(EngineCore* engineCore, ImguiEditor* editor) : editor(editor), engineCore(engineCore) {
				currentPath = ASSET_FOLDER_PATH;
				pathToRename = "";

				std::filesystem::create_directories(currentPath);

				auto textureManager = engineCore->textureManager;
				PREPARE_ICON(folder);
				PREPARE_ICON(file);
				PREPARE_ICON(image);
				PREPARE_ICON(material);
				PREPARE_ICON(model);
				PREPARE_ICON(shader);
				PREPARE_ICON(scene);
				PREPARE_ICON(sound);
				PREPARE_ICON(text);
				PREPARE_ICON(video);
			}

			ImTextureID AssetBrowserPanel::getIcon(const std::filesystem::directory_entry& directoryEntry) {
				if (directoryEntry.is_directory()) {
					return iconIds.folder;
				}
				
				const std::string& path = directoryEntry.path().string();
				size_t firstDot = path.find_last_of('.');
				std::string firstDotExtension = path.substr(firstDot);
				size_t secondDot = path.find_last_of('.', firstDot - 1);
				std::string secondDotExtension = path.substr(secondDot);

				if (firstDotExtension == ".glsl") {
					return iconIds.shader;
				}
				else if (
					firstDotExtension == ".jpg" ||
					firstDotExtension == ".jpeg" ||
					firstDotExtension == ".tga" ||
					firstDotExtension == ".bmp" ||
					firstDotExtension == ".png"
					) {
					return iconIds.image;
				}
				else if (
					firstDotExtension == ".fbx" ||
					firstDotExtension == ".obj" ||
					firstDotExtension == ".dae"
					) {
					return iconIds.model;
				}
				else if (firstDotExtension == ".gmat") {
					return iconIds.material;
				}
				else if (secondDotExtension == ".scene.json") {
					return iconIds.scene;
				}
				else if (firstDotExtension == ".txt") {
					return iconIds.text;
				}
				
				return iconIds.file;
			}
			
			void AssetBrowserPanel::setPath(std::filesystem::path path) {
				currentPath = path;
				pathToRename = "";
				refreshAssets();
			}

			void AssetBrowserPanel::refreshAssetsIfNecessary() {
				auto currentTime = std::chrono::system_clock::now();
				std::chrono::duration<double> elapsedSeconds = currentTime - lastRefreshedAssetsTime;
				if (elapsedSeconds.count() > REFRESH_INTERVAL) {
					refreshAssets();
				}
			}

			void AssetBrowserPanel::sortAlphabetically(std::vector<std::filesystem::directory_entry> entries) {
				std::sort(
					entries.begin(),
					entries.end(),
					[](const std::filesystem::directory_entry lhsEntry, const std::filesystem::directory_entry rhsEntry) {
						const auto lhs = lhsEntry.path().filename().string();
						const auto rhs = rhsEntry.path().filename().string();
						const auto result = std::mismatch(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const unsigned char lhs, const unsigned char rhs) { return tolower(lhs) == tolower(rhs); });

						return result.second != rhs.cend() &&
							(result.first == lhs.cend() || tolower(*result.first) < tolower(*result.second));
					}
				);
			}

			void AssetBrowserPanel::refreshAssets() {
				sortedDirectories.clear();
				sortedFiles.clear();

				for (const auto& directoryEntry : std::filesystem::directory_iterator(currentPath)) {
					if (directoryEntry.is_directory()) {
						sortedDirectories.push_back(directoryEntry);
					}
					else {
						sortedFiles.push_back(directoryEntry);
					}
				}

				sortAlphabetically(sortedDirectories);
				sortAlphabetically(sortedFiles);

				auto currentTime = std::chrono::system_clock::now();
				lastRefreshedAssetsTime = currentTime;
			}

			void AssetBrowserPanel::processDirectoryEntryClicks(std::filesystem::directory_entry entry) {
				if (ImGui::IsItemHovered()) {
					if (ImGui::IsMouseDoubleClicked(0)) {
						if (entry.is_directory()) {
							setPath(entry.path());
						}
						else {
							// Open File
							auto window = engineCore->windowManager->GetWindowByIndex(0);
							window->OpenFileUsingDefaultProgram(entry.path().string().c_str());
						}
					}
					else if (ImGui::IsMouseClicked(0)) {
						if (!entry.is_directory()) {
							auto path = entry.path();
							auto extension = path.extension();
							if (extension == ".gmat") {
								Manager::GetInstance().GetSelection().SetSelectedFile(path);
								return;
							}
						}
						Manager::GetInstance().GetSelection().Clear();
					}
				}
			}

			void AssetBrowserPanel::renderPath() {
				std::string path = currentPath.string();
				ImGui::Text("Path:");
				size_t previousSlash = 2;
				size_t newSlash = path.find('\\', 3);
				while(newSlash != -1) {
					std::string pathSoFar = path.substr(0, newSlash);
					std::string pathPart = path.substr(previousSlash + 1, newSlash - previousSlash - 1) + "##PathPart";
					ImGui::SameLine();
					if (ImGui::Button(pathPart.c_str())) {
						setPath(pathSoFar);
					}
					ImGui::SameLine();
					ImGui::Text(">");

					previousSlash = newSlash;
					newSlash = path.find('\\', newSlash + 1);
				}
				ImGui::SameLine();
				std::string finalPart = path.substr(previousSlash + 1, newSlash - previousSlash - 1);
				ImGui::Text(finalPart.c_str());
			}

			void AssetBrowserPanel::renderContextMenuConvertButton(std::filesystem::directory_entry entry) {
				if (entry.is_directory()) {
					return;
				}

				auto path = entry.path().string();
				size_t firstDot = path.find_last_of('.');
				std::string firstDotExtension = path.substr(firstDot);
				// size_t secondDot = path.find_last_of('.', firstDot - 1);
				// std::string secondDotExtension = path.substr(secondDot);

				if (firstDotExtension == ".glsl") {
					if (ImGui::MenuItem("Convert")) {
						Grindstone::Converters::ImportShadersFromGlsl(path.c_str());
					}
				}
				else if (
					firstDotExtension == ".jpg" ||
					firstDotExtension == ".jpeg" ||
					firstDotExtension == ".tga" ||
					firstDotExtension == ".bmp" ||
					firstDotExtension == ".png"
				) {
					if (ImGui::MenuItem("Convert")) {
						Grindstone::Converters::ImportTexture(path.c_str());
					}
				}
				else if (
					firstDotExtension == ".fbx" ||
					firstDotExtension == ".obj" ||
					firstDotExtension == ".dae"
				) {
					if (ImGui::MenuItem("Convert")) {
						Grindstone::Converters::ImportModel(path.c_str());
					}
				}
			}

			void AssetBrowserPanel::renderAssetContextMenu(std::filesystem::directory_entry entry) {
				if (ImGui::BeginPopupContextItem()) {
					auto path = entry.path();
					renderContextMenuConvertButton(entry);
					if (ImGui::MenuItem("Rename")) {
						pathToRename = path;
						pathRenameNewName = entry.path().filename().string();
					}
					if (ImGui::MenuItem("Delete")) {
						std::filesystem::remove_all(path);
						refreshAssets();
					}
					ImGui::Separator();
					if (ImGui::BeginMenu("Copy path to file")) {
						auto window = engineCore->windowManager->GetWindowByIndex(0);
						if (ImGui::MenuItem("Copy relative path")) {
							window->CopyStringToClipboard(path.string());
						}
						if (ImGui::MenuItem("Copy absolute path")) {
							auto absolutePath = std::filesystem::absolute(path);
							window->CopyStringToClipboard(absolutePath.string());
						}
						ImGui::EndMenu();
					}

					auto menuItemNameForOpen = entry.is_directory()
						? "Open folder in explorer"
						: "Open using default program";
					if (ImGui::MenuItem(menuItemNameForOpen)) {
						auto window = engineCore->windowManager->GetWindowByIndex(0);
						window->OpenFileUsingDefaultProgram(path.string().c_str());
					}
					if (ImGui::MenuItem("Reaveal in explorer")) {
						auto window = engineCore->windowManager->GetWindowByIndex(0);
						window->ExplorePath(path.string().c_str());
					}
					ImGui::EndPopup();
				}
			}

			void AssetBrowserPanel::renderCurrentDirectoryContextMenu() {
				if (ImGui::BeginPopupContextWindow()) {
					if (ImGui::BeginMenu("Create")) {
						if (ImGui::MenuItem("New Folder")) {
							std::filesystem::path newFolderName = GetNewDefaultPath(currentPath, "New folder", "");
							std::filesystem::create_directory(newFolderName);
							afterCreate(newFolderName);
						}
						if (ImGui::MenuItem("Material")) {
							auto materialPath = CreateDefaultMaterial(currentPath);
							afterCreate(materialPath);
						}
						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Import File...")) {
						editor->importFile(currentPath.string().c_str());
					}
					ImGui::Separator();
					if (ImGui::BeginMenu("Copy path to folder")) {
						auto window = engineCore->windowManager->GetWindowByIndex(0);
						if (ImGui::MenuItem("Copy relative path")) {
							window->CopyStringToClipboard(currentPath.string());
						}
						if (ImGui::MenuItem("Copy absolute path")) {
							auto absolutePath = std::filesystem::absolute(currentPath);
							window->CopyStringToClipboard(absolutePath.string());
						}
						ImGui::EndMenu();
					}
					if (ImGui::MenuItem("Open folder in explorer")) {
						auto window = engineCore->windowManager->GetWindowByIndex(0);
						window->ExplorePath(currentPath.string().c_str());
					}
					ImGui::EndPopup();
				}
			}

			void AssetBrowserPanel::afterCreate(std::filesystem::path path) {
				pathToRename = path;
				pathRenameNewName = path.filename().string();
				refreshAssets();
			}

			void AssetBrowserPanel::tryRenameFile() {
				std::filesystem::path newPath = currentPath / pathRenameNewName;
				if (!std::filesystem::exists(newPath)) {
					try {
						std::filesystem::rename(pathToRename, newPath);
						refreshAssets();
						pathToRename = "";
						pathRenameNewName = "";
					}
					catch (std::filesystem::filesystem_error error) {
						std::cerr << error.what() << std::endl;
					}
				}
				else {
					std::cerr << "This name is already used." << std::endl;
				}
			}

			void AssetBrowserPanel::renderAssetSet(std::vector<std::filesystem::directory_entry> entries) {
				for (const auto& directoryEntry : entries) {
					ImGui::TableNextColumn();

					const auto& path = directoryEntry.path();
					std::string filenameString = path.filename().string();
					std::string buttonString = filenameString + "##AssetButton";

					ImTextureID icon = getIcon(directoryEntry); 
					ImGui::PushID(buttonString.c_str());
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + THUMBNAIL_SPACING);
					ImGui::ImageButton(icon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE }, ImVec2{0,0}, ImVec2{1,1}, THUMBNAIL_PADDING);
					ImGui::PopID();

					renderAssetContextMenu(directoryEntry);
					processDirectoryEntryClicks(directoryEntry);

					if (pathToRename == path) {
						ImGui::PushItemWidth(ENTRY_SIZE);
						const auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
						if (ImGui::InputText("##AssetRename", &pathRenameNewName, flags)) {
							tryRenameFile();
						}
					}
					else {
						ImGui::TextWrapped(filenameString.c_str());
					}
				}
			}

			void AssetBrowserPanel::renderAssets() {
				const float cellSize = ENTRY_SIZE + PADDING;

				const float panelWidth = ImGui::GetContentRegionAvail().x;
				int columnCount = (int)(panelWidth / cellSize);

				if (columnCount < 1) {
					columnCount = 1;
				}

				refreshAssetsIfNecessary();

				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.05));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.1));
				if (ImGui::BeginTable("assetTable", columnCount)) {
					renderAssetSet(sortedDirectories);
					renderAssetSet(sortedFiles);

					ImGui::EndTable();
				}
				ImGui::PopStyleColor(3);
			}
			
			void AssetBrowserPanel::render() {
				if (isShowingPanel) {
					ImGui::Begin("Asset Browser", &isShowingPanel);

					renderCurrentDirectoryContextMenu();

					renderPath();

					ImGui::Separator();

					renderAssets();

					if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
						pathToRename = "";
						pathRenameNewName = "";
					}

					ImGui::End();
				}
			}
		}
	}
}
