#include "Skybox.hpp"
#include "SMaterial.hpp"
#include <string>
#include "../Core/exception.hpp"

void Skybox::Initialize(MaterialManager *material_system, GraphicsWrapper *graphics_wrapper, VertexArrayObject *plane_vao, VertexBuffer *plane_vbo) {
    material_system_ = material_system;
    graphics_wrapper_ = graphics_wrapper;
    
	plane_vao_ = plane_vao;
	plane_vbo_ = plane_vbo;

	enabled_ = false;
}

void Skybox::SetMaterial(std::string path) {
	enabled_ = true;
	try {
		material = material_system_->CreateMaterial(geometry_info_, "../assets/" + path, true);
	} catch(std::runtime_error &e) {
		enabled_ = false;
		G_WARNING(e.what());
	}
}

void Skybox::Render() {
    if (enabled_) {
		PipelineReference pipeline_reference = material.pipelineReference;
		PipelineContainer *pc = material_system_->GetPipeline(pipeline_reference);
		TextureBinding *tb = pc->materials[0].m_textureBinding;

		pc->program->Bind();
		if (tb != nullptr) {
			graphics_wrapper_->BindTextureBinding(tb);

			graphics_wrapper_->BindVertexArrayObject(plane_vao_);
			graphics_wrapper_->DrawImmediateVertices(0, 6);
		}
	}
}