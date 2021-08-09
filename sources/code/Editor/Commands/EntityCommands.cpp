#include "EntityCommands.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
using namespace Grindstone::Editor;

AddEntityCommand::AddEntityCommand(SceneManagement::Scene* scene) : scene(scene) {
	Redo();
}

void AddEntityCommand::Redo() {
	auto entity = scene->CreateEntity();
	entityId = entity.GetHandle();
}

void AddEntityCommand::Undo() {
	scene->DestroyEntity(entityId);
}

void DeleteEntityCommand::Redo() {
	// inverseCommand.Undo();
}

void DeleteEntityCommand::Undo() {
	// inverseCommand.Do();
}