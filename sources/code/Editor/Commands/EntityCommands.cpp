#include "EntityCommands.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "EngineCore/CoreComponents/Transform/TransformComponent.hpp"
#include <EngineCore/WorldContext/WorldContextSet.hpp>
using namespace Grindstone::Editor;

AddEntityCommand::AddEntityCommand(Grindstone::WorldContextSet* cxtSet) : cxtSet(cxtSet) {
	Redo();
}

void AddEntityCommand::Redo() {
	entt::registry& registry = cxtSet->GetEntityRegistry();
	entt::entity entity = registry.create();
	registry.emplace<TagComponent>(entity).tag = "Unnamed Entity";
	registry.emplace<TransformComponent>(entity);
	registry.emplace<ParentComponent>(entity);
	entityId = entity;
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
