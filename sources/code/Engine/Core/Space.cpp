#include "Space.hpp"

#include "Utilities.hpp"
#include <iostream>

#include "../Systems/LightSpotSystem.hpp"
#include "../Systems/LightPointSystem.hpp"
#include "../Systems/LightDirectionalSystem.hpp"
#include "../Systems/TransformSystem.hpp"
#include "../Systems/CameraSystem.hpp"
#include "../Systems/RenderStaticMeshSystem.hpp"
#include "../Systems/RenderTerrainSystem.hpp"
#include "../Systems/ControllerSystem.hpp"
#include "../Systems/ColliderSystem.hpp"
#include "../Systems/RigidBodySystem.hpp"
#include "../Systems/CubemapSystem.hpp"
#include "../Systems/ScriptSystem.hpp"
#include "../Systems/RenderSpriteSystem.hpp"
#include "Engine.hpp"
#include "Scene.hpp"

#include <glm/gtc/quaternion.hpp>

Space::Space(const char *name) : name_(name) {
	memset(subsystems_, 0, sizeof(subsystems_));
	addSubsystem(new ControllerSubSystem(this));
	addSubsystem(new ColliderSubSystem(this));
	addSubsystem(new RigidBodySubSystem(this));
	addSubsystem(new RenderStaticMeshSubSystem(this));
	addSubsystem(new LightPointSubSystem(this));
	addSubsystem(new LightSpotSubSystem(this));
	addSubsystem(new LightDirectionalSubSystem(this));
	addSubsystem(new RenderSpriteSubSystem(this));
	addSubsystem(new TransformSubSystem(this));
	addSubsystem(new CubemapSubSystem(this));
	addSubsystem(new CameraSubSystem(this));
	addSubsystem(new ScriptSubSystem(this));
	addSubsystem(new RenderTerrainSubSystem((RenderTerrainSystem *)engine.getSystem(COMPONENT_RENDER_TERRAIN), this));
}

Space::Space(const Space &s) {
	name_ = s.name_;

	for (int i = 0; i < NUM_COMPONENTS; ++i) {
		/*if (s.subsystems_[i]) {
			subsystems_[i] = s.subsystems_[i]->copy();
		}
		else*/
		subsystems_[i] = nullptr;
	}
}

void Space::clear() {
	for (int i = 0; i < NUM_COMPONENTS; ++i) {
		if (subsystems_[i])
			delete subsystems_[i];
	}

	memset(subsystems_, 0, sizeof(subsystems_));
}

Space::~Space() {
	clear();
}

SubSystem * Space::getSubsystem(ComponentType type) {
	return subsystems_[type];
}

Scene* Space::getScene(std::string levelname) {
	for (Scene *s : scenes_) {
		if (s->getName() == levelname) {
			return s;
		}
	}

	return nullptr;
}

Scene* Space::getScene(unsigned int id) {
	return scenes_[id];
}

std::string Space::getName() {
	return name_;
}

Scene* Space::addScene(const char *levelname) {
	return scenes_.emplace_back(new Scene(this, levelname));
}

SubSystem *Space::addSubsystem(SubSystem * system) {
	subsystems_[system->getSystemType()] = system;
	return system;
}

size_t Space::getNumObjects() {
	return objects_.size();
}

GameObjectHandle Space::createObject(const char *name) {
	auto id = (GameObjectHandle)objects_.size();
	objects_.emplace_back(id, name, -1);

	return id;
}