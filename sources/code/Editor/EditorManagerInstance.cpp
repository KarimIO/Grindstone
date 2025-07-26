#include <filesystem>
#include <functional>
#include "EditorManager.hpp"

static Grindstone::Editor::Manager* editorManagerInstance = nullptr;

Grindstone::Editor::Manager& Grindstone::Editor::Manager::GetInstance() {
	return *editorManagerInstance;
}

void Grindstone::Editor::Manager::SetInstance(Grindstone::Editor::Manager* editorManager) {
	editorManagerInstance = editorManager;
}
