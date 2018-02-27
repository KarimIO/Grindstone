#include "Skybox.hpp"
#include "SMaterial.hpp"

void Skybox::Initialize(MaterialManager *material_system, GraphicsWrapper *graphics_wrapper) {
    material_system_ = material_system;
    graphics_wrapper_ = graphics_wrapper;
    material = material_system_->CreateMaterial(geometry_info_, "../assets/materials/skymain.gmat", true);

    float planeVerts[4 * 6] = {
		-1.0, -1.0,
		1.0, -1.0,
		-1.0,  1.0,
		1.0,  1.0,
		-1.0,  1.0,
		1.0, -1.0,
	};

    VertexBufferCreateInfo planeVboCI;
	planeVboCI.binding = geometry_info_.vbds;
	planeVboCI.bindingCount = 1;
	planeVboCI.attribute = geometry_info_.vads;
	planeVboCI.attributeCount = 1;
	planeVboCI.content = planeVerts;
	planeVboCI.count = 6;
	planeVboCI.size = sizeof(float) * 6 * 2;

	VertexArrayObjectCreateInfo vaci;
	vaci.vertexBuffer = planeVBO;
	vaci.indexBuffer = nullptr;
	planeVAO = graphics_wrapper_->CreateVertexArrayObject(vaci);
	planeVBO = graphics_wrapper_->CreateVertexBuffer(planeVboCI);

    vaci.vertexBuffer = planeVBO;
	vaci.indexBuffer = nullptr;
	planeVAO->BindResources(vaci);
	planeVAO->Unbind();
}

void Skybox::Render() {
    PipelineReference pipeline_reference = material.pipelineReference;
    PipelineContainer *pc = material_system_->GetPipeline(pipeline_reference);
    TextureBinding *tb = pc->materials[0].m_textureBinding;

    pc->program->Bind();
    if (tb != nullptr) {
        graphics_wrapper_->BindTextureBinding(tb);

        graphics_wrapper_->BindVertexArrayObject(planeVAO);
		graphics_wrapper_->DrawImmediateVertices(0, 6);
    }
}