#include <algorithm>
#include <fstream>
#include <string>
#include <cctype>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>

#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/Window/WindowManager.hpp>
#include <Editor/ImguiEditor/ImguiEditor.hpp>
#include <Editor/EditorManager.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Scenes/Manager.hpp>

#include "ImguiRenderer.hpp"
#include "AssetBrowserPanel.hpp"

using namespace Grindstone::Editor::ImguiEditor;

const double REFRESH_INTERVAL = 1.0;
const float PADDING = 8.0f;
const float ENTRY_SIZE = 80.0f;
const float THUMBNAIL_SIZE = 64.0f;
const float THUMBNAIL_PADDING = 4.0f;
const float THUMBNAIL_SPACING = (ENTRY_SIZE - THUMBNAIL_SIZE - THUMBNAIL_PADDING) / 2.0f;

static std::filesystem::path GetNewDefaultPath(const std::filesystem::path& basePath, std::string fileName, std::string extension) {
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

AssetBrowserPanel::AssetBrowserPanel(
	ImguiRenderer* imguiRenderer,
	EngineCore* engineCore,
	ImguiEditor* editor
) : editor(editor),
	engineCore(engineCore),
	rootDirectory(Editor::Manager::GetFileManager().GetPrimaryDirectory()) {
	pathToRename = "";

	iconIds.folderIcon = imguiRenderer->CreateTexture("assetIcons/Folder");

	for (uint16_t i = 0; i < static_cast<uint16_t>(AssetType::Count); ++i) {
		auto assetType = static_cast<AssetType>(i);

		iconIds.fileIcons[i] = imguiRenderer->CreateTexture(std::string("assetIcons/") + std::string(GetAssetTypeToString(assetType)));
	}

	currentDirectory = &rootDirectory;
}

ImTextureID AssetBrowserPanel::GetIcon(const AssetType assetType) const {
	uint16_t uintType = static_cast<uint16_t>(assetType);
	if (uintType >= static_cast<uint16_t>(AssetType::Count)) {
		return iconIds.fileIcons[static_cast<uint16_t>(AssetType::Undefined)];
	}

	return iconIds.fileIcons[uintType];
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
	auto& path = entry.path();
	if (ImGui::IsItemHovered()) {
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (entry.is_directory() && directory) {
				SetCurrentAssetDirectory(*directory);
			}
			else if (path.extension().string() == ".gscene") {
				Editor::Manager& engineManager = Editor::Manager::GetInstance();
				SceneManagement::SceneManager* sceneManager = engineManager.GetEngineCore().GetSceneManager();
				std::filesystem::path relativePath = std::filesystem::relative(path, engineManager.GetAssetsPath());

				AssetRegistry& assetRegistry = engineManager.GetAssetRegistry();

				AssetRegistry::Entry entry;
				if (assetRegistry.TryGetAssetData(relativePath, entry)) {
					sceneManager->LoadScene(entry.uuid);
				}
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

	auto& path = entry.path();
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
			importerManager.Import(path);
			Editor::Manager::GetInstance().GetAssetRegistry().WriteFile();
		}
	}

	// TODO: Get uuids from meta file so I can reload them
}

void AssetBrowserPanel::RenderAssetContextMenu(std::filesystem::directory_entry entry) {
	if (ImGui::BeginPopupContextItem()) {
		auto& path = entry.path();
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

	auto& currentPath = currentDirectory->path.path();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	if (ImGui::BeginPopupContextWindow()) {
		if (ImGui::BeginMenu("Create")) {
			if (ImGui::MenuItem("New Folder")) {
				std::filesystem::path newFolderName = GetNewDefaultPath(currentPath, "New folder", "");
				std::filesystem::create_directory(newFolderName);
				AfterCreate(newFolderName);
			}
			RenderAssetTemplates(currentPath);
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

void AssetBrowserPanel::RenderAssetTemplates(const std::filesystem::path& path) {
	AssetTemplateRegistry& registry = Editor::Manager::GetInstance().GetAssetTemplateRegistry();
	for (AssetTemplateRegistry::AssetTemplate& assetTemplate : registry) {
		if (ImGui::MenuItem(assetTemplate.name.c_str())) {
			std::string newFileName = "New " + assetTemplate.name;
			std::filesystem::path finalFilePath = GetNewDefaultPath(
				path,
				newFileName.c_str(),
				assetTemplate.extension
			);

			std::ofstream output(finalFilePath, std::ios::binary);
			output.write(assetTemplate.content.data(), assetTemplate.content.size());
			output.close();

			AfterCreate(finalFilePath);
		}
	}
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
			GPRINT_ERROR_V(LogSource::Editor, "Rename failed: %s!", error.what());
		}
	}
	else {
		GPRINT_ERROR_V(LogSource::Editor, "Could not rename file, this name is already used.");
	}
}

void AssetBrowserPanel::RenderFolders() {
	if (currentDirectory == nullptr) {
		return;
	}

	for (Directory* subdirectory : currentDirectory->subdirectories) {
		auto& directoryEntry = subdirectory->path;
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

		ImTextureID icon = iconIds.folderIcon;
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

	bool hasDefaultAsset = false;
	MetaFile::Subasset subasset;
	AssetType assetType = AssetType::Undefined;
	Uuid myUuid;
	if (file->metaFile.TryGetDefaultSubasset(subasset)) {
		hasDefaultAsset = true;
		assetType = subasset.assetType;
		myUuid = subasset.uuid;
	}

	ImTextureID icon = GetIcon(assetType);
	ImGui::PushID(buttonString.c_str());
	ImGui::SetCursorPosX(cursorX + THUMBNAIL_SPACING);
	ImGui::ImageButton(icon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE }, ImVec2{ 0,0 }, ImVec2{ 1,1 }, (int)THUMBNAIL_PADDING);
	ImVec2 rectMin = ImGui::GetItemRectMin();
	ImVec2 rectMax = ImGui::GetItemRectMin();
	ImGui::PopID();

	RenderAssetContextMenu(file->directoryEntry);
	ProcessDirectoryEntryClicks(file->directoryEntry);

	if (hasDefaultAsset && ImGui::BeginDragDropSource()) {
		std::string myUuidAsString = myUuid.ToString();
		std::string assetTypeStr = GetAssetTypeToString(assetType);
		ImGui::SetDragDropPayload(assetTypeStr.c_str(), &myUuid, sizeof(Uuid));
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
				ImTextureID icon = GetIcon(subasset.assetType);
				buttonString = filenameString + subasset.uuid.ToString();
				ImGui::PushID(buttonString.c_str());
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + THUMBNAIL_SPACING);
				ImGui::ImageButton(icon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE }, ImVec2{ 0,0 }, ImVec2{ 1,1 }, (int)THUMBNAIL_PADDING);
				ImGui::PopID();

				if (ImGui::BeginDragDropSource()) {
					Uuid myUuid = subasset.uuid;
					std::string myUuidAsString = myUuid.ToString();
					ImGui::SetDragDropPayload("_UUID", myUuidAsString.data(), myUuidAsString.size() + 1);
					ImGui::Text(subasset.displayName.c_str());
					ImGui::EndDragDropSource();
				}

				ImGui::TextWrapped(subasset.displayName.c_str());
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
