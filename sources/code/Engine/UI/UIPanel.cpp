#include <Engine/Core/Engine.hpp>
#include <Engine/AssetManagers/MaterialManager.hpp>
#include <Engine/UI/UIPanel.hpp>

UIPanel::UIPanel(std::string material_path) {
	
}

void UIPanel::updateLayout() {
	Quad2D &q2d = r2d_->getQuadMemory(quad_id_);
	q2d.vertices[0].position.x = q2d.vertices[1].position.x = left_;
	q2d.vertices[0].position.y = q2d.vertices[3].position.y = top_;
	q2d.vertices[2].position.x = q2d.vertices[3].position.x = right_;
	q2d.vertices[1].position.y = q2d.vertices[2].position.y = bottom_;
	r2d_->updateBuffers();
}

UIPanel::~UIPanel() {

}
