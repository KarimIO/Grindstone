#include "ComponentCommands.hpp"
#include "EngineCore/Scenes/Scene.hpp"
using namespace Grindstone::Editor;

AddComponentCommand::AddComponentCommand(
	SceneManagement::Scene* scene, entt::entity entityId, const char* componentName
) : scene(scene), entityId(entityId), componentName(componentName) {
	scene->attachComponent(entityId, componentName);
}

void AddComponentCommand::Redo() {
	scene->attachComponent(entityId, componentName.c_str());
}

void AddComponentCommand::Undo() {
	scene->detachComponent(entityId, componentName.c_str());
}

void DeleteComponentCommand::Redo() {
}

void DeleteComponentCommand::Undo() {
}