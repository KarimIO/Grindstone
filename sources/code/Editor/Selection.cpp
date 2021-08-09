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

void Selection::RemoveEntity(ECS::Entity entity) {
	selectedEntities.erase(selectedEntities.find(entity));
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

void Selection::SetSelectedFile(std::filesystem::path path) {
	Clear();
	AddFile(path);
}

void Selection::AddFile(std::filesystem::path path) {
	selectedFiles.insert(path);
}

void Selection::RemoveFile(std::filesystem::path path) {
	selectedFiles.erase(selectedFiles.find(path));
}

size_t Selection::GetSelectedFileCount() {
	return selectedFiles.size();
}

std::filesystem::path Selection::GetSingleSelectedFile() {
	return *selectedFiles.begin();
}
