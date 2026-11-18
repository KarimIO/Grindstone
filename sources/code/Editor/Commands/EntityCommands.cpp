#include <EngineCore/CoreComponents/Parent/ParentComponent.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>
#include "EntityCommands.hpp"

using namespace Grindstone::Editor;

AddEntityCommand::AddEntityCommand(Grindstone::WorldContextSet* cxtSet) : cxtSet(cxtSet) {
	Redo();
}

void AddEntityCommand::Redo() {
	entt::registry& registry = cxtSet->GetEntityRegistry();
	entityId = registry.create();
	registry.emplace<Grindstone::TagComponent>(entityId).tag = "Unnamed";
	registry.emplace<Grindstone::TransformComponent>(entityId);
	registry.emplace<Grindstone::ParentComponent>(entityId);
}

void AddEntityCommand::Undo() {
	cxtSet->GetEntityRegistry().destroy(entityId);
}

void DeleteEntityCommand::Redo() {
	// inverseCommand.Undo();
}

void DeleteEntityCommand::Undo() {
	// inverseCommand.Do();
}
