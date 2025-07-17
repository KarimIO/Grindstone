#include "ComponentCommands.hpp"
#include "EngineCore/Scenes/Scene.hpp"
using namespace Grindstone::Editor;

AddComponentCommand::AddComponentCommand(ECS::Entity entity, Grindstone::HashedString componentName)
	: entity(entity), componentName(componentName) {
	entity.AddComponent(componentName);
}

void AddComponentCommand::Redo() {
	entity.AddComponent(componentName);
}

void AddComponentCommand::Undo() {
	entity.RemoveComponent(componentName);
}

void DeleteComponentCommand::Redo() {
}

void DeleteComponentCommand::Undo() {
}
