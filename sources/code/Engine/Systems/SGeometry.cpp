#include "SGeometry.h"

void SGeometry::AddComponent(GeometryType type, std::string path) {
	switch (type) {
	case GEOMETRY_STATIC_MODEL:
		break;
	case GEOMETRY_SKELETAL_MODEL:
		break;
	case GEOMETRY_TERRAIN:
		break;
	}
}

void SGeometry::AddSystem(SSubGeometry *system) {
	systems.push_back(system);
}

void SGeometry::LoadPreloaded() {
	for (auto &system : systems) {
		system->LoadPreloaded();
	}
}

SGeometry::~SGeometry() {
	for (auto &system : systems) {
		delete system;
	}
}