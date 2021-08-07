#include "EntityCommands.hpp"
#include "EngineCore/Scenes/Scene.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
using namespace Grindstone::Editor;

AddEntityCommand::AddEntityCommand(SceneManagement::Scene* scene) : scene(scene) {
	entityId = scene->getEntityRegistry()->create();
	auto tag = (TagComponent*)scene->attachComponent(entityId, "Tag");
	tag->tag = "New Component";
}

void AddEntityCommand::Redo() {
	scene->getEntityRegistry()->create(entityId);
	auto tag = (TagComponent*)scene->attachComponent(entityId, "Tag");
	tag->tag = "New Component";
}

void AddEntityCommand::Undo() {
	scene->getEntityRegistry()->destroy(entityId);
}

void DeleteEntityCommand::Redo() {
	// inverseCommand.Undo();
}

void DeleteEntityCommand::Undo() {
	// inverseCommand.Do();
}