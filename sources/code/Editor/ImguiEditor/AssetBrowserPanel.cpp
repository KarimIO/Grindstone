#include <algorithm>
#include <fstream>
#include <string>
#include <cctype>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <entt/entt.hpp>

#include <EditorCommon/ResourcePipeline/MetaFile.hpp>
#include <Common/ResourcePipeline/AssetType.hpp>
#include <Common/Window/WindowManager.hpp>
#include <Editor/ImguiEditor/ImguiEditor.hpp>
#include <Editor/EditorManager.hpp>
#include <Editor/PluginSystem/EditorPluginManager.hpp>
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

static void RenderAssetElement(bool isSelected, const char* buttonString, float cursorX, ImTextureID icon) {
	ImVec4 mainColor = isSelected ? ImGui::GetStyle().Colors[ImGuiCol_Button] : ImVec4(1.f, 1.f, 1.f, 0.f);
	ImVec4 hoveredColor = isSelected ? ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] : ImVec4(1.f, 1.f, 1.f, 0.05f);
	ImVec4 activeColor = isSelected ? ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] : ImVec4(1.f, 1.f, 1.f, 0.1f);
	ImGui::PushStyleColor(ImGuiCol_Button, mainColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
	ImGui::PushID(buttonString);
	ImGui::SetCursorPosX(cursorX + THUMBNAIL_SPACING);
	ImGui::ImageButton(icon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE }, ImVec2{ 0,0 }, ImVec2{ 1,1 }, (int)THUMBNAIL_PADDING);
	ImGui::PopID();
	ImGui::PopStyleColor(3);
}

AssetBrowserPanel::AssetBrowserPanel(
	ImguiRenderer* imguiRenderer,
	EngineCore* engineCore,
	ImguiEditor* editor
) : editor(editor),
	engineCore(engineCore),
	assetBrowserInspectType(AssetBrowserInspectType::All)
{
	auto& fileManager = Editor::Manager::GetFileManager();
	SetAllPlugins();
	indexToRename = SIZE_MAX;

	iconIds.folderIcon = imguiRenderer->CreateTexture("assetIcons/Folder");

	for (uint16_t i = 0; i < static_cast<uint16_t>(AssetType::Count); ++i) {
		AssetType assetType = static_cast<AssetType>(i);

		iconIds.fileIcons[i] = imguiRenderer->CreateTexture(std::string("assetIcons/") + std::string(GetAssetTypeToString(assetType)));
	}
}

ImTextureID AssetBrowserPanel::GetIcon(const AssetType assetType) const {
	uint16_t uintType = static_cast<uint16_t>(assetType);
	if (uintType >= static_cast<uint16_t>(AssetType::Count)) {
		return iconIds.fileIcons[static_cast<uint16_t>(AssetType::Undefined)];
	}

	return iconIds.fileIcons[uintType];
}

void AssetBrowserPanel::AddFilePath(const std::filesystem::directory_entry& file) {
	const std::filesystem::path& path = file.path();
	if (file.is_directory()) {
		folders.emplace_back(path);
	}
	else if (path.extension().string() != ".meta") {
		AssetRegistry& registry = Editor::Manager::GetInstance().GetAssetRegistry();
		AssetType assetType = AssetType::Undefined;
		AssetRegistry::Entry outEntry;
		if (registry.TryGetAssetDataFromAbsolutePath(path, outEntry)) {
			assetType = outEntry.assetType;
		}
		ImTextureID icon = GetIcon(assetType);
		Grindstone::Editor::MetaFile metaFile(registry, path);
		Uuid defaultSubassetUuid;
		AssetType defaultSubassetType = AssetType::Undefined;
		Editor::MetaFile::Subasset defaultSubasset;
		if (metaFile.TryGetDefaultSubasset(defaultSubasset)) {
			defaultSubassetUuid = defaultSubasset.uuid;
			defaultSubassetType = defaultSubasset.assetType;
		}

		auto& newItem = files.emplace_back(AssetBrowserItem{ path, path.filename(), defaultSubassetType, defaultSubasset.displayName, defaultSubassetUuid });

		for (const MetaFile::Subasset& defaultSubasset : metaFile) {
			newItem.subassets.emplace_back(
				AssetBrowserItem::Subasset{
					defaultSubasset.uuid,
					defaultSubasset.displayName,
					defaultSubasset.assetType
				}
			);
		}
	}
}

void AssetBrowserPanel::SetAllPlugins() {
	assetBrowserInspectType = AssetBrowserInspectType::All;
	currentMountingPoint = nullptr;
	currentDirectory = std::filesystem::directory_entry();
	searchText = "";
	Editor::Manager::GetInstance().GetSelection().ClearFiles();
	folders.clear();
	files.clear();
}

void AssetBrowserPanel::SetCurrentPlugin(const std::string& pluginName) {
	assetBrowserInspectType = AssetBrowserInspectType::Plugin;
	currentMountingPoint = nullptr;
	currentDirectory = std::filesystem::directory_entry(pluginName);
	searchText = "";
	Editor::Manager::GetInstance().GetSelection().ClearFiles();
	folders.clear();
	files.clear();
}

void AssetBrowserPanel::SetCurrentAssetDirectory(const std::filesystem::path& path) {
	assetBrowserInspectType = AssetBrowserInspectType::Directory;
	Editor::FileManager& fileManager = Editor::Manager::GetInstance().GetFileManager();
	const Editor::FileManager::MountPoint* newMountPoint = nullptr;
	for (const Editor::FileManager::MountPoint& mountPoint : fileManager.GetMountedDirectories()) {
		std::string relative = std::filesystem::relative(path, mountPoint.path).string();
		if (relative.size() != 0 && (relative.size() == 1 || relative[0] != '.' && relative[1] != '.')) {
			newMountPoint = &mountPoint;
		}
	}

	if (newMountPoint == nullptr) {
		return;
	}

	currentMountingPoint = newMountPoint;
	currentDirectory = std::filesystem::directory_entry(path);
	searchText = "";
	Editor::Manager::GetInstance().GetSelection().ClearFiles();

	SetFilesFromCurrentDirectory();
}

void AssetBrowserPanel::SetFilesFromCurrentDirectory() {
	indexToRename = SIZE_MAX;
	folders.clear();
	files.clear();

	auto folderIterator = static_cast<std::filesystem::directory_iterator>(currentDirectory);
	for (const std::filesystem::directory_entry& file : folderIterator) {
		AddFilePath(file);
	}
}

void AssetBrowserPanel::ProcessFolderClicks(const std::filesystem::path& path) {
	if (ImGui::IsItemHovered()) {
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			SetCurrentAssetDirectory(path);
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			Selection& selection = Editor::Manager::GetInstance().GetSelection();
			if (ImGui::GetIO().KeyCtrl) {
				if (selection.IsFileSelected(path)) {
					selection.RemoveFile(path);
				}
				else {
					selection.AddFile(path);
				}
			}
			else {
				selection.SetSelectedFile(path);
			}
		}
	}
}

void AssetBrowserPanel::ProcessFileClicks(AssetBrowserItem& item) {
	if (ImGui::IsItemHovered()) {
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			if (item.defaultAssetType == AssetType::Scene) {
				Editor::Manager& engineManager = Editor::Manager::GetInstance();
				SceneManagement::SceneManager* sceneManager = engineManager.GetEngineCore().GetSceneManager();

				AssetRegistry& assetRegistry = engineManager.GetAssetRegistry();

				AssetRegistry::Entry entry;
				if (item.defaultUuid.IsValid()) {
					sceneManager->LoadScene(item.defaultUuid);
				}
			}
			else {
				auto window = engineCore->windowManager->GetWindowByIndex(0);
				window->OpenFileUsingDefaultProgram(item.filepath.string().c_str());
			}
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			Selection& selection = Editor::Manager::GetInstance().GetSelection();
			if (ImGui::GetIO().KeyCtrl) {
				if (selection.IsFileSelected(item.filepath)) {
					selection.RemoveFile(item.filepath);
				}
				else {
					selection.AddFile(item.filepath);
				}
			}
			else {
				selection.SetSelectedFile(item.filepath);
			}
		}
	}
}

void AssetBrowserPanel::RenderTopBar() {
	if (assetBrowserInspectType != AssetBrowserInspectType::Directory) {
		return;
	}

	ImGui::BeginChildFrame(ImGui::GetID("#assetpathbar"), ImVec2(0, 32), ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);
	
	RenderPath();

	float totalAvailWidth = ImGui::GetContentRegionAvail().x;
	const float searchWidth = 160.0f;
	const float searchX = totalAvailWidth - searchWidth;

	ImGui::SameLine(totalAvailWidth - 160.0f);
	ImGui::PushItemWidth(searchWidth);
	if (ImGui::InputTextWithHint("##Search", "Search", &searchText)) {
		if (searchText.empty()) {
			searchTextLower = "";
			SetFilesFromCurrentDirectory();
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

void AssetBrowserPanel::RenderPathPart(const std::filesystem::path& path) {
	const std::filesystem::path& parent = path.parent_path();
	if (path != currentMountingPoint->path && parent != currentMountingPoint->path) {
		RenderPathPart(parent);
	}

	std::string pathPart = path.filename().string() + "##PathPart";
	if (ImGui::Button(pathPart.c_str())) {
		SetCurrentAssetDirectory(std::filesystem::directory_entry(path));
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
	const std::filesystem::path& path = currentDirectory.path();
	if (path != currentMountingPoint->path) {
		RenderPathPart(path.parent_path());
		ImGui::SameLine();
	}
	std::string finalPart = path.filename().string() + "##PathPart";
	ImGui::Button(finalPart.c_str());
	ImGui::PopStyleColor(3);
}

void AssetBrowserPanel::RenderContextMenuFileTypeSpecificEntries(const std::filesystem::path& path) {
	auto pathStr = path.string();
	size_t firstDot = pathStr.find_last_of('.') + 1;
	std::string firstDotExtension = pathStr.substr(firstDot);
	EngineCore& engineCore = Editor::Manager::GetEngineCore();

	auto& importerManager = Editor::Manager::GetInstance().GetImporterManager();
	auto importerFactory = importerManager.GetImporterFactoryByExtension(firstDotExtension).factory;
	if (importerFactory != nullptr) {
		if (ImGui::MenuItem("Import")) {
			importerManager.Import(path);
			Editor::Manager::GetInstance().GetAssetRegistry().WriteFile();
		}
	}

	// TODO: Get uuids from meta file so I can reload them
}

void AssetBrowserPanel::RenderAssetContextMenu(bool isFolder, const std::filesystem::path& path, size_t index) {
	if (ImGui::BeginPopupContextItem()) {
		if (!isFolder) {
			RenderContextMenuFileTypeSpecificEntries(path);
		}

		if (ImGui::MenuItem("Rename")) {
			isRenamingFolder = isFolder;
			indexToRename = index;
			pathRenameNewName = path.filename().string();
		}
		if (ImGui::MenuItem("Delete")) {
			std::filesystem::remove_all(path);
			// TODO: This should happen in an event callback
			if (isFolder) {
				folders.erase(folders.begin() + index);
			}
			else {
				files.erase(files.begin() + index);
			}
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

		auto menuItemNameForOpen = isFolder
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
	if (currentDirectory.path().empty()) {
		return;
	}

	const std::filesystem::path& currentPath = currentDirectory.path();
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
template <typename T>
void MoveRange(size_t start, size_t length, size_t dst, std::vector<T>& v) {
	typename std::vector<T>::iterator first, middle, last;
	if (start < dst)
	{
		first = v.begin() + start;
		middle = first + length;
		last = v.begin() + dst;
	}
	else
	{
		first = v.begin() + dst;
		middle = v.begin() + start;
		last = middle + length;
	}
	std::rotate(first, middle, last);
}

template<typename T>
void MoveItemInVector(std::vector<T>& vec, size_t fromIndex, size_t toIndex) {
	T item = vec[fromIndex];
	vec.insert(vec.begin() + toIndex, item);
	if (fromIndex > toIndex) {
		vec.erase(vec.begin() + fromIndex + 1);
	}
	else {
		vec.erase(vec.begin() + fromIndex);
	}
}

size_t AssetBrowserPanel::SortFile(bool isFolder, size_t indexToSort) {
	if (isFolder) {
		std::filesystem::path pathToSort = folders[indexToSort];
		std::string pathAsStr = pathToSort.filename().string();
		std::transform(
			pathAsStr.begin(),
			pathAsStr.end(),
			pathAsStr.begin(),
			::tolower
		);
		size_t newIndex = folders.size();
		for (size_t index = 0; index < folders.size(); ++index) {
			std::string itemNameLower = folders[index].filename().string();
			std::transform(
				itemNameLower.begin(),
				itemNameLower.end(),
				itemNameLower.begin(),
				::tolower
			);
			if (indexToSort != index && itemNameLower > pathAsStr) {
				newIndex = index;
				break;
			}
		}

		MoveItemInVector(folders, indexToSort, newIndex);
		return newIndex;
	}
	else {
		AssetBrowserItem fileToSort = files[indexToSort];
		const std::filesystem::path& pathToSort = fileToSort.filename;
		std::string pathAsStr = pathToSort.string();
		std::transform(
			pathAsStr.begin(),
			pathAsStr.end(),
			pathAsStr.begin(),
			::tolower
		);
		size_t newIndex = 0;
		for (size_t index = 0; index < files.size(); ++index) {
			std::string itemNameLower = files[index].filename.string();
			std::transform(
				itemNameLower.begin(),
				itemNameLower.end(),
				itemNameLower.begin(),
				::tolower
			);
			if (indexToSort != index && itemNameLower > pathAsStr) {
				newIndex = index;
				break;
			}
		}

		MoveItemInVector(files, indexToSort, newIndex);
		return newIndex;
	}
}

void AssetBrowserPanel::AfterCreate(const std::filesystem::path& path) {
	const std::filesystem::directory_entry entry = std::filesystem::directory_entry(path);
	AddFilePath(entry);
	pathRenameNewName = path.filename().string();
	if (entry.is_directory()) {
		isRenamingFolder = true;
		indexToRename = SortFile(isRenamingFolder, folders.size() - 1);
	}
	else {
		isRenamingFolder = false;
		indexToRename = SortFile(isRenamingFolder, files.size() - 1);
	}
}

void AssetBrowserPanel::TryRenameFile() {
	std::filesystem::path newPath = currentDirectory.path() / pathRenameNewName;

	std::filesystem::path& pathToRename = isRenamingFolder
		? folders[indexToRename]
		: files[indexToRename].filepath;

	if (std::filesystem::exists(newPath)) {
		auto& fileItem = isRenamingFolder
			? folders[indexToRename]
			: files[indexToRename].filepath;
		if (fileItem == newPath) {
			indexToRename = SIZE_MAX;
			pathRenameNewName = "";
			return;
		}
		else {
			newPath = GetNewDefaultPath(
				newPath.parent_path(),
				newPath.filename().replace_extension("").string(),
				newPath.filename().extension().string()
			);
		}
	}

	try {
		std::filesystem::rename(pathToRename, newPath);

		if (isRenamingFolder) {
			pathToRename = newPath;
		}
		else {
			AssetBrowserItem& fileItem = files[indexToRename];
			fileItem.filename = newPath.filename();
			fileItem.filepath = newPath;
		}
		SortFile(isRenamingFolder, indexToRename);

		indexToRename = SIZE_MAX;
		pathRenameNewName = "";
	}
	catch (const std::filesystem::filesystem_error& error) {
		GPRINT_ERROR_V(LogSource::Editor, "Rename failed: %s!", error.what());
	}
}

void AssetBrowserPanel::RenderAllPlugins() {
	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::Plugins::EditorPluginManager& pluginManager = static_cast<Grindstone::Plugins::EditorPluginManager&>(*engineCore.GetPluginManager());

	for (Grindstone::Plugins::MetaData& plugin : pluginManager) {
		ImGui::TableNextColumn();

		std::string buttonString = plugin.name + "##AssetButton";

		float cursorX = ImGui::GetCursorPosX();
		ImTextureID icon = iconIds.folderIcon;
		RenderAssetElement(false, buttonString.c_str(), cursorX, icon);
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
			SetCurrentPlugin(plugin.name);
		}
		ImGui::TextWrapped(plugin.name.c_str());
	}
}

void AssetBrowserPanel::RenderPlugins() {
	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::Plugins::EditorPluginManager& pluginManager = static_cast<Grindstone::Plugins::EditorPluginManager&>(*engineCore.GetPluginManager());

	for (Grindstone::Plugins::MetaData& plugin : pluginManager) {
		// TODO: String comparison is slow, don't do that.
		if (plugin.name != currentDirectory.path().string()) {
			continue;
		}

		for (Grindstone::Plugins::MetaData::AssetDirectory& asset : plugin.assetDirectories) {
			ImGui::TableNextColumn();

			std::string assetDirectoryString = asset.assetDirectoryRelativePath.string();
			std::string buttonString = assetDirectoryString + "##AssetButton";
			RenderAssetElement(false, buttonString.c_str(), ImGui::GetCursorPosX(), iconIds.fileIcons[0]);
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
				std::filesystem::path path(plugin.pluginResolvedPath / asset.assetDirectoryRelativePath);
				SetCurrentAssetDirectory(path);
			}
			ImGui::TextWrapped(assetDirectoryString.c_str());
		}

		for (Grindstone::Plugins::MetaData::Binary& binary : plugin.binaries) {
			ImGui::TableNextColumn();

			std::filesystem::path path(plugin.pluginResolvedPath / binary.libraryRelativePath);
			std::filesystem::directory_entry directoryEntry(path);

			ImTextureID icon = iconIds.fileIcons[1];
			switch (binary.buildType) {
			default:
			case Plugins::MetaData::BinaryBuildType::NoBuild:
				icon = iconIds.fileIcons[1];
				break;
			case Plugins::MetaData::BinaryBuildType::Dotnet:
				icon = iconIds.fileIcons[2];
				break;
			case Plugins::MetaData::BinaryBuildType::Cmake:
				icon = iconIds.fileIcons[3];
				break;
			}

			std::string buttonString = binary.buildTarget + "##AssetButton";
			RenderAssetElement(false, buttonString.c_str(), ImGui::GetCursorPosX(), icon);
			ImGui::TextWrapped(binary.buildTarget.c_str());
		}

		return;
	}
}

void AssetBrowserPanel::RenderFolders() {
	for (size_t folderIndex = 0; folderIndex < folders.size(); ++folderIndex) {
		const std::filesystem::path& path = folders[folderIndex];
		ImGui::TableNextColumn();

		std::string filenameString = path.filename().string();
		std::string buttonString = filenameString + "##AssetButton";

		bool isSelected = Editor::Manager::GetInstance().GetSelection().IsFileSelected(path);
		float cursorX = ImGui::GetCursorPosX();
		ImTextureID icon = iconIds.folderIcon;
		RenderAssetElement(isSelected, buttonString.c_str(), cursorX, icon);

		RenderAssetContextMenu(true, path, folderIndex);
		ProcessFolderClicks(path);


		if (isRenamingFolder && indexToRename == folderIndex) {
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
	for (size_t index = 0; index < files.size(); ++index) {
		RenderFile(index);
	}
}

void AssetBrowserPanel::RenderFile(size_t fileIndex) {
	ImGui::TableNextColumn();
	AssetBrowserItem& item = files[fileIndex];

	std::string filename = item.filename.string();
	std::string buttonString = filename + "##AssetButton";

	bool isSelected = Editor::Manager::GetInstance().GetSelection().IsFileSelected(item.filepath);

	float cursorX = ImGui::GetCursorPosX();
	float cursorY = ImGui::GetCursorPosY();
	AssetType assetType = item.defaultAssetType;
	ImTextureID icon = GetIcon(assetType);

	RenderAssetElement(isSelected, buttonString.c_str(), cursorX, icon);
	RenderAssetContextMenu(false, item.filepath, fileIndex);
	ProcessFileClicks(item);

	if (item.defaultUuid.IsValid() && ImGui::BeginDragDropSource()) {
		std::string myUuidAsString = item.defaultUuid.ToString();
		std::string assetTypeStr = GetAssetTypeToString(assetType);
		ImGui::SetDragDropPayload(assetTypeStr.c_str(), &item.defaultUuid, sizeof(Grindstone::Uuid));
		ImGui::Text("%s\n%s\n%s", item.defaultAssetName.c_str(), filename.c_str(), assetTypeStr.c_str());
		ImGui::EndDragDropSource();
	}

	if (!isRenamingFolder && indexToRename == fileIndex) {
		ImGui::PushItemWidth(ENTRY_SIZE);
		const auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
		if (ImGui::InputText("##AssetRename", &pathRenameNewName, flags)) {
			TryRenameFile();
		}
	}
	else {
		ImGui::TextWrapped(filename.c_str());
	}

	if (item.subassets.size() > 0) {
		bool isExpanded = item.isSubassetListOpen;
		ImGui::SetCursorPosX(cursorX + THUMBNAIL_SIZE + THUMBNAIL_PADDING);
		ImGui::SetCursorPosY(cursorY + (THUMBNAIL_SIZE / 2.0f) - 5.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
		std::string expandButton = isExpanded
			? std::string("<##") + buttonString
			: std::string(">##") + buttonString;

		if (ImGui::Button(expandButton.c_str(), ImVec2(20, 20))) {
			item.isSubassetListOpen = !item.isSubassetListOpen;
		}

		ImGui::PopStyleVar();

		ImGui::SetCursorPosX(cursorX);
		ImGui::SetCursorPosY(cursorY);

		if (isExpanded) {
			for (const AssetBrowserItem::Subasset& subasset : item.subassets) {
				ImGui::TableNextColumn();
				ImTextureID icon = GetIcon(subasset.assetType);
				buttonString = item.filepath.string() + subasset.uuid.ToString();
				ImGui::PushID(buttonString.c_str());
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + THUMBNAIL_SPACING);
				ImGui::ImageButton(icon, { THUMBNAIL_SIZE, THUMBNAIL_SIZE }, ImVec2{ 0,0 }, ImVec2{ 1,1 }, (int)THUMBNAIL_PADDING);
				ImGui::PopID();

				if (ImGui::BeginDragDropSource()) {
					Uuid myUuid = subasset.uuid;
					std::string myUuidAsString = myUuid.ToString();
					std::string subAssetTypeStr = GetAssetTypeToString(subasset.assetType);
					ImGui::SetDragDropPayload(subAssetTypeStr.c_str(), &subasset.uuid, sizeof(Grindstone::Uuid));
					ImGui::Text("%s\n%s\n%s", subasset.name.c_str(), filename.c_str(), subAssetTypeStr.c_str());
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

	if (assetBrowserInspectType == AssetBrowserInspectType::Directory && folders.empty() && files.empty()) {
		if (!searchText.empty()) {
			ImGui::Text("Search could not find any results.");
		}
		else {
			ImGui::Text("This folder is empty.");
		}
	}
	else if (ImGui::BeginTable("##assetTable", columnCount, ImGuiTableFlags_NoSavedSettings)) {
		switch (assetBrowserInspectType) {
		case AssetBrowserInspectType::All:
			RenderAllPlugins();
			break;
		case AssetBrowserInspectType::Plugin:
			RenderPlugins();
			break;
		case AssetBrowserInspectType::Directory:
			RenderFolders();
			RenderFiles();
			break;
		}

		ImGui::EndTable();
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
		if (!ImGui::GetIO().KeyCtrl) {
			Selection& selection = Editor::Manager::GetInstance().GetSelection();
			selection.Clear();
		}

		indexToRename = SIZE_MAX;
		pathRenameNewName = "";
	}

	ImGui::EndChildFrame();
}

void AssetBrowserPanel::RenderSidebar(float height) {
	auto sidebarId = ImGui::GetID("#assetSidebar");
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	ImGui::BeginChildFrame(sidebarId, ImVec2(0, height), ImGuiWindowFlags_NoBackground);
	ImGui::PopStyleVar();

	Editor::FileManager& fileManager = Editor::Manager::GetFileManager();
	const auto& mountPoints = fileManager.GetMountedDirectories();

	if (mountPoints.empty()) {
		ImGui::Text("This folder is empty.");
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	Grindstone::Plugins::EditorPluginManager& pluginManager = static_cast<Grindstone::Plugins::EditorPluginManager&>(*engineCore.GetPluginManager());
	for (Grindstone::Plugins::MetaData& plugin : pluginManager) {
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;

		bool hasChildren = !plugin.assetDirectories.empty() || !plugin.binaries.empty();

		if (!hasChildren) {
			nodeFlags |= ImGuiTreeNodeFlags_Leaf;
		}

		if (currentDirectory.path().string() == plugin.name) {
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
		}

		if (ImGui::TreeNodeEx(plugin.name.c_str(), nodeFlags)) {
			if (ImGui::IsItemClicked()) {
				SetCurrentPlugin(plugin.name);
			}

			for (Grindstone::Plugins::MetaData::AssetDirectory& asset : plugin.assetDirectories) {
				std::filesystem::path path(plugin.pluginResolvedPath / asset.assetDirectoryRelativePath);
				std::filesystem::directory_entry directoryEntry(path);
				RenderSidebarSubdirectory(directoryEntry);
			}

			for (Grindstone::Plugins::MetaData::Binary& binary : plugin.binaries) {
				std::filesystem::path path(plugin.pluginResolvedPath / binary.libraryRelativePath);
				std::filesystem::directory_entry directoryEntry(path);

				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf;
				if (ImGui::TreeNodeEx(binary.buildTarget.c_str(), nodeFlags)) {
					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}
		else if (ImGui::IsItemClicked()) {
			SetCurrentPlugin(plugin.name);
		}
	}

	ImGui::EndChildFrame();
}

void AssetBrowserPanel::RenderSidebarSubdirectory(const std::filesystem::directory_entry& entry) {
	std::string path = entry.path().filename().string();

	if (path.empty()) {
		return;
	}

	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
	bool hasChildren = false;
	for (const std::filesystem::directory_entry& child : std::filesystem::directory_iterator(entry)) {
		if (child.is_directory()) {
			hasChildren = true;
		}
	}

	if (!hasChildren) {
		nodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	if (entry == currentDirectory) {
		nodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (ImGui::TreeNodeEx(path.c_str(), nodeFlags)) {
		if (ImGui::IsItemClicked()) {
			SetCurrentAssetDirectory(entry);
		}

		for (const std::filesystem::directory_entry& child : std::filesystem::directory_iterator(entry)) {
			if (child.is_directory()) {
				RenderSidebarSubdirectory(child);
			}
		}

		ImGui::TreePop();
	}
	else if (ImGui::IsItemClicked()) {
		SetCurrentAssetDirectory(entry);
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

void AssetBrowserPanel::FilterSearch() {
	// TODO: Mutex this up and put it in a job so we don't have to wait for
	// - it to search potentially a lot of files. We can search N amount of
	// - files, and every N files we push the files to the vectors until
	// - everything is processed
	indexToRename = std::numeric_limits<size_t>().max();
	folders.clear();
	files.clear();
	for (const auto& entry : std::filesystem::recursive_directory_iterator(currentDirectory)) {
		std::string filename = entry.path().filename().string();
		std::transform(
			filename.begin(),
			filename.end(),
			filename.begin(),
			::tolower
		);

		if (filename.find(searchTextLower) != std::string::npos) {
			AddFilePath(entry);
		}
	}
}
