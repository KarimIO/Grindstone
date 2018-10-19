/*#include "SGeometry.hpp"

void SGeometry::AddComponent(uint32_t entityID, uint32_t &component, GeometryType type) {
	component = render_components_.size();
	render_components_.push_back(CRender());
	CRender &r = render_components_.back();
	r.entity_id = entityID;
	r.geometry_type = type;
}

void SGeometry::RemoveComponent(uint32_t id) {
	// CRender &r = render_components_[id];
	// systems_[r.geometry_type]->RemoveGeometryInstance(r.geometry_id);
	// Swap r with back, and size--
}

CRender &SGeometry::GetComponent(uint32_t id) {
	return render_components_[id];
}

void SGeometry::AddSystem(SSubGeometry *system) {
	systems_.push_back(system);
}

SSubGeometry *SGeometry::GetSystem(uint32_t id) {
	return systems_[id];
}

void SGeometry::LoadPreloaded() {
	for (auto &system : systems_) {
		system->LoadPreloaded();
	}
}

void SGeometry::Cull(CCamera *cam) {
	for (auto &system : systems_) {
		system->Cull(cam);
	}
}

SGeometry::~SGeometry() {
	cleanup();
}

void SGeometry::cleanup() {
	for (auto &system : systems_) {
		delete system;
	}

	systems_.clear();
}*/