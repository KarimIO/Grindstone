#include <algorithm>
#include <string>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <entt/entt.hpp>
#include "ComponentInspector.hpp"
#include "AssetBrowserPanel.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/EngineCore.hpp"

const std::filesystem::path assetFolderPath = "..\\assets";
const double refreshInterval = 1.0;
const bool RIGHT_MOUSE_BUTTON = 1;

namespace Grindstone {
	namespace Editor {
		namespace ImguiEditor {
			AssetBrowserPanel::AssetBrowserPanel(EngineCore* engineCore) : engineCore(engineCore) {
				currentPath = assetFolderPath;
				pathToRename = "";
			}
			
			void AssetBrowserPanel::setPath(std::filesystem::path path) {
				currentPath = path;
				pathToRename = "";
				refreshAssets();
			}

			void AssetBrowserPanel::refreshAssetsIfNecessary() {
				auto currentTime = std::chrono::system_clock::now();
				std::chrono::duration<double> elapsedSeconds = currentTime - lastRefreshedAssetsTime;
				if (elapsedSeconds.count() > refreshInterval) {
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

			void AssetBrowserPanel::clickDirectoryEntry(std::filesystem::directory_entry entry) {
				if (entry.is_directory()) {
					setPath(entry.path());
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

			void AssetBrowserPanel::renderAssetContextMenu(std::filesystem::directory_entry entry) {
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Rename")) {
						pathToRename = entry.path();
						pathRenameNewName = entry.path().filename().string();
					}
					if (ImGui::MenuItem("Delete")) {
						std::filesystem::remove_all(entry.path());
						refreshAssets();
					}
					ImGui::EndPopup();
				}
			}

			void AssetBrowserPanel::renderCurrentDirectoryContextMenu() {
				if (ImGui::BeginPopupContextWindow()) {
					if (ImGui::MenuItem("Create New Folder")) {
						std::filesystem::path newFolderName = currentPath / "New folder";
						std::filesystem::create_directory(newFolderName);
						pathToRename = newFolderName;
						pathRenameNewName = newFolderName.filename().string();
						refreshAssets();
					}
					ImGui::EndPopup();
				}
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
					ImGui::Button(buttonString.c_str(), { thumbnailSize, thumbnailSize });
					renderAssetContextMenu(directoryEntry);
					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
						clickDirectoryEntry(directoryEntry);
					}

					if (pathToRename == path) {
						ImGui::PushItemWidth(thumbnailSize);
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
				const float cellSize = thumbnailSize + padding;

				const float panelWidth = ImGui::GetContentRegionAvail().x;
				int columnCount = (int)(panelWidth / cellSize);

				if (columnCount < 1) {
					columnCount = 1;
				}

				refreshAssetsIfNecessary();

				if (ImGui::BeginTable("assetTable", columnCount)) {
					renderAssetSet(sortedDirectories);
					renderAssetSet(sortedFiles);

					ImGui::EndTable();
				}
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
