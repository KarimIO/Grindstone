#include "Selection.hpp"
using namespace Grindstone::Editor;

void Selection::Clear() {
	ClearEntities();
	ClearFiles();
}

void Selection::ClearEntities() {
	selectedEntities.clear();
}

void Selection::SetSelectedEntity(ECS::Entity entity) {
	Clear();
	AddEntity(entity);
}

void Selection::AddEntity(ECS::Entity entity) {
	selectedEntities.insert(entity);
}

bool Selection::IsEntitySelected(ECS::Entity entity) {
	return selectedEntities.find(entity) != selectedEntities.end();
}

void Selection::RemoveEntity(ECS::Entity entity) {
	auto& selectedEntity = selectedEntities.find(entity);
	if (selectedEntity != selectedEntities.end()) {
		selectedEntities.erase(selectedEntity);
	}
}

size_t Selection::GetSelectedEntityCount() {
	return selectedEntities.size();
}

Grindstone::ECS::Entity Selection::GetSingleSelectedEntity() {
	return *selectedEntities.begin();
}

void Selection::ClearFiles() {
	selectedFiles.clear();
}

void Selection::SetSelectedFile(const std::filesystem::path& path) {
	Clear();
	AddFile(path);
}

void Selection::AddFile(const std::filesystem::path& path) {
	selectedFiles.insert(path);
}

bool Selection::IsFileSelected(const std::filesystem::path& path) {
	return selectedFiles.find(path) != selectedFiles.end();
}

void Selection::RemoveFile(const std::filesystem::path& path) {
	selectedFiles.erase(selectedFiles.find(path));
}

size_t Selection::GetSelectedFileCount() {
	return selectedFiles.size();
}

std::filesystem::path Selection::GetSingleSelectedFile() {
	return *selectedFiles.begin();
}
