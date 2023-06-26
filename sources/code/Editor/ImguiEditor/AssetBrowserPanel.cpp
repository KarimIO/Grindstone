#include <algorithm>
#include <fstream>
#include <string>
#include <cctype>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>
#include "ComponentInspector.hpp"
#include "Editor/ImguiEditor/ImguiEditor.hpp"
#include "Editor/Importers/ShaderImporter.hpp"
#include "Editor/Importers/ModelImporter.hpp"
#include "Editor/Importers/TextureImporter.hpp"
#include "Editor/EditorManager.hpp"
#include "AssetBrowserPanel.hpp"
#include "Common/Window/WindowManager.hpp"
#include "EngineCore/Scenes/Manager.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/Assets/Textures/TextureImporter.hpp"
#include "EngineCore/Assets/Shaders/ShaderImporter.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "Plugins/GraphicsOpenGL/GLTexture.hpp"
using namespace Grindstone::Editor::ImguiEditor;

const double REFRESH_INTERVAL = 1.0;
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

ImTextureID GetIdFromTexture(GraphicsAPI::Texture* texture) {
	GraphicsAPI::GLTexture* glTex = (GraphicsAPI::GLTexture*)texture;
	return (ImTextureID)(uint64_t)glTex->GetTexture();
}

void PrepareIcon(Grindstone::Assets::AssetManager* assetManager, const char* path, GraphicsAPI::Texture*& texture, ImTextureID& id) {
	auto textureAsset = static_cast<TextureAsset*>(assetManager->GetAsset(Grindstone::AssetType::Texture, path));

	if (textureAsset == nullptr) {
		return;
	}

	texture = textureAsset->texture;
	id = GetIdFromTexture(texture);
}

#define PREPARE_ICON(type) PrepareIcon(assetManager, "../engineassets/editor/assetIcons/" #type ".dds", iconTextures.type, iconIds.type)

AssetBrowserPanel::AssetBrowserPanel(EngineCore* engineCore, ImguiEditor* editor) : editor(editor), engineCore(engineCore), rootDirectory(Editor::Manager::GetFileManager().GetRootDirectory()) {
	pathToRename = "";

	auto assetManager = engineCore->assetManager;
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

	currentDirectory = &rootDirectory;
}

ImTextureID AssetBrowserPanel::GetIcon(const std::filesystem::directory_entry& directoryEntry) {
	if (directoryEntry.is_directory()) {
		return iconIds.folder;
	}

	const std::string& path = directoryEntry.path().string();
	size_t firstDot = path.find_last_of('.');
	std::string firstDotExtension = path.substr(firstDot);
	size_t secondDot = path.find_last_of('.', firstDot - 1);
	std::string secondDotExtension = secondDot == std::string::npos ? "" : path.substr(secondDot);

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

void AssetBrowserPanel::SetCurrentAssetDirectory(Directory& newDirectory) {
	currentDirectory = &newDirectory;
	pathToRename = "";
	searchText = "";
	searchedFiles.clear();
	Editor::Manager::GetInstance().GetSelection().ClearFiles();
	expandedAssetUuidsWithSubassets.clear();
}

void AssetBrowserPanel::ProcessDirectoryEntryClicks(std::filesystem::directory_entry entry, Directory* directory) {
	auto path = entry.path();
	if (ImGui::IsItemHovered()) {
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (entry.is_directory() && directory) {
				SetCurrentAssetDirectory(*directory);
			}
			else {
				auto window = engineCore->windowManager->GetWindowByIndex(0);
				window->OpenFileUsingDefaultProgram(path.string().c_str());
			}
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			Selection& selection = Editor::Manager::GetInstance().GetSelection();
			if (ImGui::GetIO().KeyCtrl) {
				if (selection.IsFileSelected(entry)) {
					selection.RemoveFile(entry);
				}
				else {
					selection.AddFile(entry);
				}
			}
			else {
				selection.SetSelectedFile(entry);
			}
		}
	}
}

void AssetBrowserPanel::RenderTopBar() {
	ImGui::BeginChildFrame(ImGui::GetID("#assetpathbar"), ImVec2(0, 32), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
	
	RenderPath();

	float totalAvailWidth = ImGui::GetContentRegionAvail().x;
	const float searchWidth = 160.0f;
	const float searchX = totalAvailWidth - searchWidth;

	ImGui::SameLine(totalAvailWidth - 160.0f);
	ImGui::PushItemWidth(searchWidth);
	if (ImGui::InputTextWithHint("##Search", "Search", &searchText)) {
		if (searchText.empty()) {
			currentDirectory = &rootDirectory;
		}
		else {
			searchTextLower.resize(searchText.size());
			std::transform(
				searchText.begin(),
				searchText.end(),
				searchTextLower.begin(),
				::tolower
			);
			FilterSearch();
		}
	}
	ImGui::EndChildFrame();
}

void AssetBrowserPanel::RenderPathPart(Directory* directory) {
	if (directory == nullptr) {
		return;
	}

	RenderPathPart(directory->parentDirectory);

	std::string pathPart = directory->path.path().filename().string() + "##PathPart";
	if (ImGui::Button(pathPart.c_str())) {
		SetCurrentAssetDirectory(*directory);
	}
	ImGui::SameLine();
	ImGui::Text("/");
	ImGui::SameLine();
}

void AssetBrowserPanel::RenderPath() {
	if (!searchText.empty()) {
		ImGui::Text("Showing search results:");
		return;
	}

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.15f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
	RenderPathPart(currentDirectory->parentDirectory);
	ImGui::SameLine();
	std::string finalPart = currentDirectory->path.path().filename().string() + "##PathPart";
	ImGui::Button(finalPart.c_str());
	ImGui::PopStyleColor(3);
}

void AssetBrowserPanel::RenderContextMenuFileTypeSpecificEntries(std::filesystem::directory_entry entry) {
	if (entry.is_directory()) {
		return;
	}

	auto path = entry.path();
	auto pathStr = path.string();
	size_t firstDot = pathStr.find_last_of('.') + 1;
	std::string firstDotExtension = pathStr.substr(firstDot);
	EngineCore& engineCore = Editor::Manager::GetEngineCore();
	// size_t secondDot = path.find_last_of('.', firstDot - 1);
	// std::string secondDotExtension = path.substr(secondDot);

	auto& importerManager = Editor::Manager::GetInstance().GetImporterManager();
	auto importerFactory = importerManager.GetImporterFactoryByExtension(firstDotExtension);
	if (importerFactory != nullptr) {
		if (ImGui::MenuItem("Import")) {
			importerFactory(path);
		}
	}

	// TODO: Get uuids from meta file so I can reload them
	/*
	if (firstDotExtension == "glsl") {
		if (ImGui::MenuItem("Reload")) {
			std::string pathWithoutExtension = pathStr.substr(0, firstDot);
			engineCore.shaderImporter->ReloadShaderIfLoaded(pathWithoutExtension.c_str());
		}
	}
	else if (firstDotExtension == "gmat") {
		if (ImGui::MenuItem("Reload")) {
			engineCore.shaderImporter->ReloadMaterialIfLoaded(pathStr.c_str());
		}
	}
	else if (firstDotExtension == "dds") {
		if (ImGui::MenuItem("Reload")) {
			engineCore.shaderImporter->ReloadTextureIfLoaded(pathStr.c_str());
		}
	}
	*/
}

void AssetBrowserPanel::RenderAssetContextMenu(std::filesystem::directory_entry entry) {
	if (ImGui::BeginPopupContextItem()) {
		auto path = entry.path();
		RenderContextMenuFileTypeSpecificEntries(entry);
		if (ImGui::MenuItem("Rename")) {
			pathToRename = path;
			pathRenameNewName = entry.path().filename().string();
		}
		if (ImGui::MenuItem("Delete")) {
			std::filesystem::remove_all(path);
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
		if (ImGui::MenuItem("Reveal in explorer")) {
			auto window = engineCore->windowManager->GetWindowByIndex(0);
			window->ExplorePath(path.string().c_str());
		}
		ImGui::EndPopup();
	}
}

void AssetBrowserPanel::RenderCurrentDirectoryContextMenu() {
	if (currentDirectory == nullptr) {
		return;
	}

	auto currentPath = currentDirectory->path.path();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::BeginMenu("Create")) {
			if (ImGui::MenuItem("New Folder")) {
				std::filesystem::path newFolderName = GetNewDefaultPath(currentPath, "New folder", "");
				std::filesystem::create_directory(newFolderName);
				AfterCreate(newFolderName);
			}
			if (ImGui::MenuItem("Material")) {
				auto materialPath = CreateDefaultMaterial(currentPath);
				AfterCreate(materialPath);
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Import File...")) {
			editor->ImportFile(currentPath.string().c_str());
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
	ImGui::PopStyleVar();
}

void AssetBrowserPanel::AfterCreate(std::filesystem::path path) {
	pathToRename = path;
	pathRenameNewName = path.filename().string();
}

void AssetBrowserPanel::TryRenameFile() {
	std::filesystem::path newPath = currentDirectory->path / pathRenameNewName;
	if (!std::filesystem::exists(newPath)) {
		try {
			std::filesystem::rename(pathToRename, newPath);
			pathToRename = "";
			pathRenameNewName = "";
		}
		catch (std::filesystem::filesystem_error error) {
			Editor::Manager::Print(LogSeverity::Error, "Rename failed: %s!", error.what());
		}
	}
	else {
		Editor::Manager::Print(LogSeverity::Error, "Could not rename file, this name is already used.");
	}
}

void AssetBrowserPanel::RenderFolders() {
	if (currentDirectory == nullptr) {
		return;
	}

	for (Directory* subdirectory : currentDirectory->subdirectories) {
		auto directoryEntry = subdirectory->path;
		ImGui::TableNextColumn();

		const auto& path = directoryEntry.path();
		std::string filenameString = path.filename().string();
		std::string buttonString = filenameString + "##AssetButton";

		bool isSelected = Editor::Manager::GetInstance().GetSelection().IsFileSelected(directoryEntry);
		ImVec4 mainColor = isSelected ? ImGui::GetStyle().Colors[ImGuiCol_Button] : ImVec4(1.f, 1.f, 1.f, 0.f);
		ImVec4 hoveredColor = isSelected ? ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] : ImVec4(1.f, 1.f, 1.f, 0.05f);
		ImVec4 activeColor = isSelected ? ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] : ImVec4(1.f, 1.f, 1.f, 0.1f);
		ImGui::PushStyleColor(ImGuiCol_Button, mainColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);

		ImTextureID icon = GetIcon(directoryEntry);
		ImGui::PushID(buttonString.c_str());
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + THUMBNAIL_SPACING);
		ImGui::ImageButton(icon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE }, ImVec2{ 0,0 }, ImVec2{ 1,1 }, (int)THUMBNAIL_PADDING);
		ImGui::PopID();

		RenderAssetContextMenu(directoryEntry);
		ProcessDirectoryEntryClicks(directoryEntry, subdirectory);

		ImGui::PopStyleColor(3);

		if (pathToRename == path) {
			ImGui::PushItemWidth(ENTRY_SIZE);
			const auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
			if (ImGui::InputText("##AssetRename", &pathRenameNewName, flags)) {
				TryRenameFile();
			}
		}
		else {
			ImGui::TextWrapped(filenameString.c_str());
		}
	}
}

void AssetBrowserPanel::RenderFiles() {
	if (!searchText.empty()) {
		for (auto file : searchedFiles) {
			RenderFile(file);
		}
		return;
	}

	for (auto file : currentDirectory->files) {
		RenderFile(file);
	}
}

void AssetBrowserPanel::RenderFile(File* file) {
	ImGui::TableNextColumn();

	const auto& path = file->directoryEntry.path();
	std::string filenameString = path.filename().string();
	std::string buttonString = filenameString + "##AssetButton";

	bool isSelected = Editor::Manager::GetInstance().GetSelection().IsFileSelected(file->directoryEntry);
	ImVec4 mainColor = isSelected ? ImGui::GetStyle().Colors[ImGuiCol_Button] : ImVec4(1.f, 1.f, 1.f, 0.f);
	ImVec4 hoveredColor = isSelected ? ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] : ImVec4(1.f, 1.f, 1.f, 0.05f);
	ImVec4 activeColor = isSelected ? ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] : ImVec4(1.f, 1.f, 1.f, 0.1f);
	ImGui::PushStyleColor(ImGuiCol_Button, mainColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);

	float cursorX = ImGui::GetCursorPosX();
	float cursorY = ImGui::GetCursorPosY();

	ImTextureID icon = GetIcon(file->directoryEntry);
	ImGui::PushID(buttonString.c_str());
	ImGui::SetCursorPosX(cursorX + THUMBNAIL_SPACING);
	ImGui::ImageButton(icon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE }, ImVec2{ 0,0 }, ImVec2{ 1,1 }, (int)THUMBNAIL_PADDING);
	ImVec2 rectMin = ImGui::GetItemRectMin();
	ImVec2 rectMax = ImGui::GetItemRectMin();
	ImGui::PopID();

	RenderAssetContextMenu(file->directoryEntry);
	ProcessDirectoryEntryClicks(file->directoryEntry);

	Uuid myUuid;
	if (
		file->metaFile.TryGetDefaultSubassetUuid(myUuid) &&
		ImGui::BeginDragDropSource()
	) {
		std::string myUuidAsString = myUuid.ToString();
		ImGui::SetDragDropPayload("_UUID", myUuidAsString.data(), myUuidAsString.size() + 1);
		ImGui::Text("%s", filenameString.c_str());
		ImGui::EndDragDropSource();
	}

	ImGui::PopStyleColor(3);

	if (pathToRename == path) {
		ImGui::PushItemWidth(ENTRY_SIZE);
		const auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
		if (ImGui::InputText("##AssetRename", &pathRenameNewName, flags)) {
			TryRenameFile();
		}
	}
	else {
		ImGui::TextWrapped(filenameString.c_str());
	}

	if (file->metaFile.GetSubassetCount() > 0) {
		bool isExpanded = expandedAssetUuidsWithSubassets.find(myUuid) != expandedAssetUuidsWithSubassets.end();
		ImGui::SetCursorPosX(cursorX + THUMBNAIL_SIZE + THUMBNAIL_PADDING);
		ImGui::SetCursorPosY(cursorY + (THUMBNAIL_SIZE / 2.0f) - 5.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
		std::string expandButton = isExpanded
			? std::string("<##") + buttonString
			: std::string(">##") + buttonString;

		if (ImGui::Button(expandButton.c_str(), ImVec2(20, 20))) {
			if (isExpanded) {
				expandedAssetUuidsWithSubassets.erase(myUuid);
			}
			else {
				expandedAssetUuidsWithSubassets.emplace(myUuid);
			}
		}

		ImGui::PopStyleVar();

		ImGui::SetCursorPosX(cursorX);
		ImGui::SetCursorPosY(cursorY);

		if (isExpanded) {
			for (auto& subasset : file->metaFile) {
				ImGui::TableNextColumn();
				ImTextureID icon = 0; //GetIcon(file->directoryEntry);
				buttonString = filenameString + subasset.uuid.ToString();
				ImGui::PushID(buttonString.c_str());
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + THUMBNAIL_SPACING);
				ImGui::ImageButton(icon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE }, ImVec2{ 0,0 }, ImVec2{ 1,1 }, (int)THUMBNAIL_PADDING);
				ImGui::PopID();

				if (ImGui::BeginDragDropSource()) {
					Uuid myUuid = subasset.uuid;
					std::string myUuidAsString = myUuid.ToString();
					ImGui::SetDragDropPayload("_UUID", myUuidAsString.data(), myUuidAsString.size() + 1);
					ImGui::Text(subasset.name.c_str());
					ImGui::EndDragDropSource();
				}

				ImGui::TextWrapped(subasset.name.c_str());
			}
		}
	}
}

void AssetBrowserPanel::RenderAssets(float height) {
	const float cellSize = ENTRY_SIZE + PADDING;

	const float panelWidth = ImGui::GetContentRegionAvail().x;
	int columnCount = (int)(panelWidth / cellSize);

	if (columnCount < 1) {
		columnCount = 1;
	}

	auto assetPanel = ImGui::GetID("#assetsPanel");
	ImGui::BeginChildFrame(assetPanel, ImVec2(0, height), ImGuiWindowFlags_NoBackground);
	RenderCurrentDirectoryContextMenu();

	if (currentDirectory != nullptr && currentDirectory->subdirectories.empty() && currentDirectory->files.empty()) {
		ImGui::Text("This folder is empty.");
	}
	else if (!searchText.empty() && searchedFiles.size() == 0) {
		ImGui::Text("Search could not find any results.");
	}
	else if (ImGui::BeginTable("##assetTable", columnCount, ImGuiTableFlags_NoSavedSettings)) {
		RenderFolders();
		RenderFiles();

		ImGui::EndTable();
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
		if (!ImGui::GetIO().KeyCtrl) {
			Selection& selection = Editor::Manager::GetInstance().GetSelection();
			selection.Clear();
		}
		pathToRename = "";
		pathRenameNewName = "";
	}

	ImGui::EndChildFrame();
}

void AssetBrowserPanel::RenderSidebar(float height) {
	auto sidebarId = ImGui::GetID("#assetSidebar");
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	ImGui::BeginChildFrame(sidebarId, ImVec2(0, height), ImGuiWindowFlags_NoBackground);
	ImGui::PopStyleVar();

	if (rootDirectory.subdirectories.empty()) {
		ImGui::Text("This folder is empty.");
	}
	else {
		RenderSidebarSubdirectory(rootDirectory);
	}

	ImGui::EndChildFrame();
}

void AssetBrowserPanel::RenderSidebarSubdirectory(Directory& directory) {
	std::string path = directory.path.path().filename().string();

	if (path.empty()) {
		return;
	}

	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
	if (directory.subdirectories.empty()) {
		nodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	if (currentDirectory == &directory) {
		nodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (ImGui::TreeNodeEx(path.c_str(), nodeFlags)) {
		if (ImGui::IsItemClicked()) {
			SetCurrentAssetDirectory(directory);
		}

		for (auto directory : directory.subdirectories) {
			RenderSidebarSubdirectory(*directory);
		}

		ImGui::TreePop();
	}
	else if (ImGui::IsItemClicked()) {
		SetCurrentAssetDirectory(directory);
	}
}

void AssetBrowserPanel::Render() {
	if (isShowingPanel) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
		ImGui::Begin("Asset Browser", &isShowingPanel);
		ImGui::PopStyleVar();

		RenderTopBar();

		bool isInTable = ImGui::BeginTable("assetBrowserSplit", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX);

		if (isInTable) {
			ImGui::TableNextRow();
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_TitleBg)));
			float height = ImGui::GetContentRegionAvail().y - 4.0f;
			ImGui::TableNextColumn();
			RenderSidebar(height);
			ImGui::TableNextColumn();
			RenderAssets(height);
			ImGui::EndTable();
		}

		ImGui::End();
	}
}

void AssetBrowserPanel::FilterSearch(Directory& dir) {
	for (auto file : dir.files) {
		std::string fileName = file->directoryEntry.path().filename().string();
		std::string fileNameLower;
		fileNameLower.resize(fileName.size());
		std::transform(
			fileName.begin(),
			fileName.end(),
			fileNameLower.begin(),
			::tolower
		);

		if (fileNameLower.find(searchTextLower) != std::string::npos) {
			searchedFiles.push_back(file);
		}
	}

	for (auto subdir : dir.subdirectories) {
		FilterSearch(*subdir);
	}
}

void AssetBrowserPanel::FilterSearch() {
	currentDirectory = nullptr;
	pathToRename = "";
	searchedFiles.clear();
	Editor::Manager::GetInstance().GetSelection().ClearFiles();
	expandedAssetUuidsWithSubassets.clear();

	searchedFiles.clear();
	FilterSearch(rootDirectory);
}
