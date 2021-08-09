#include "ComponentCommands.hpp"
#include "EngineCore/Scenes/Scene.hpp"
using namespace Grindstone::Editor;

AddComponentCommand::AddComponentCommand(ECS::Entity entity, const char* componentName)
	: entity(entity), componentName(componentName) {
	entity.AddComponent(componentName);
}

void AddComponentCommand::Redo() {
	entity.AddComponent(componentName.c_str());
}

void AddComponentCommand::Undo() {
	entity.RemoveComponent(componentName.c_str());
}

void DeleteComponentCommand::Redo() {
}

void DeleteComponentCommand::Undo() {
}