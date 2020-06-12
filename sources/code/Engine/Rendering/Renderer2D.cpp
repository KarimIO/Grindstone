#include <Engine/Core/Engine.hpp>
#include <Engine/Rendering/Renderer2D.hpp>

#include "AssetManagers/MaterialManager.hpp"

void Renderer2D::setMaterial(std::string path) {
	geometry_info_.ubbs = nullptr;
	geometry_info_.ubb_count = 0;
	vertex_layout_ = Grindstone::GraphicsAPI::VertexBufferLayout({
		{ Grindstone::GraphicsAPI::VertexFormat::Float2, "vertexPosition", false, Grindstone::GraphicsAPI::AttributeUsage::Position },
		{ Grindstone::GraphicsAPI::VertexFormat::Float4, "vertexColor", false, Grindstone::GraphicsAPI::AttributeUsage::Color },
		{ Grindstone::GraphicsAPI::VertexFormat::Float2, "vertexTexCoord", false, Grindstone::GraphicsAPI::AttributeUsage::TexCoord0 }
		});

	// Fix this soon
	geometry_info_.vertex_layout = &vertex_layout_;
	geometry_info_.vertex_layout_count = 1;

	material_reference_ = engine.getMaterialManager()->loadMaterial(geometry_info_, path);
	engine.getMaterialManager()->getMaterial(material_reference_)->m_meshes.push_back(this);
}

void Renderer2D::shadowDraw() {
}

void Renderer2D::draw() {
	engine.getGraphicsWrapper()->setImmediateBlending(Grindstone::GraphicsAPI::BlendMode::AdditiveAlpha);

	float clear_col[4] = { 0.3f, 0.6f, 0.9f, 1.0f };
	engine.getGraphicsPipelineManager()->getPipeline(material_reference_.pipelineReference)->program->Bind();

	Material *material = engine.getMaterialManager()->getMaterial(material_reference_);
	if (material->m_textureBinding != nullptr)
		engine.getGraphicsWrapper()->bindTextureBinding(material->m_textureBinding);

	engine.getGraphicsWrapper()->clear(Grindstone::GraphicsAPI::ClearMode::ColorAndDepth, clear_col, 1.0f, 0);
	engine.getGraphicsWrapper()->bindVertexArrayObject(vao_);
	engine.getGraphicsWrapper()->drawImmediateIndexed(Grindstone::GraphicsAPI::GeometryType::Triangles, true, 0, 0, capacity_ * 6);
}

void Renderer2D::resize(unsigned int size) {
	auto gw = engine.getGraphicsWrapper();

	Quad2D* temp_buffer = new Quad2D[size];
	unsigned int* temp_indices = new unsigned int[size * 6];
	bool* temp_free_quad_list = new bool[size];

	unsigned int c = size; // (capacity_ > size) ? size : capacity_;

	if (vertex_buffer_) {
		memcpy(temp_buffer, vertex_buffer_, sizeof(Quad2D) * c);

		delete vertex_buffer_;
	}

	if (indices_buffer_) {
		memcpy(temp_indices, indices_buffer_, sizeof(unsigned int) * 6 * c);

		delete indices_buffer_;
	}

	if (free_quad_list_) {
		memcpy(temp_free_quad_list, free_quad_list_, sizeof(bool) * c);

		delete free_quad_list_;
	}

	if (size > capacity_) {
		for (unsigned int i = capacity_; i < size; ++i) {
			unsigned int i6 = i * 6;
			unsigned int i4 = i * 4;

			temp_indices[i6 + 0] = i4 + 0;
			temp_indices[i6 + 1] = i4 + 1;
			temp_indices[i6 + 2] = i4 + 2;
			temp_indices[i6 + 3] = i4 + 2;
			temp_indices[i6 + 4] = i4 + 3;
			temp_indices[i6 + 5] = i4 + 0;

			temp_free_quad_list[i] = false;
		}
	}

	vertex_buffer_ = temp_buffer;
	indices_buffer_ = temp_indices;
	free_quad_list_ = temp_free_quad_list;

	if (vbo_) gw->deleteVertexBuffer(vbo_);
	if (ibo_) gw->deleteIndexBuffer(ibo_);
	if (vao_) gw->deleteVertexArrayObject(vao_);

	Grindstone::GraphicsAPI::VertexBufferCreateInfo vbci;
	vbci.layout = &vertex_layout_;
	vbci.content = nullptr;
	vbci.count = size * 4;
	vbci.size = sizeof(Vertex2D) * vbci.count;
	vbo_ = gw->createVertexBuffer(vbci);

	Grindstone::GraphicsAPI::IndexBufferCreateInfo ibci;
	ibci.content = indices_buffer_;
	ibci.count = size * 6;
	ibci.size = sizeof(unsigned int) * ibci.count;
	ibo_ = gw->createIndexBuffer(ibci);

	Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo vaoci;
	vaoci.vertex_buffers = &vbo_;
	vaoci.vertex_buffer_count = 1;
	vaoci.index_buffer = ibo_;
	vao_ = gw->createVertexArrayObject(vaoci);
	
	capacity_ = size;
}

unsigned int Renderer2D::requestQuadSlot() {
	for (unsigned int i = 0; i < capacity_; ++i) {
		if (free_quad_list_[i]) return i;
	}
	return -1;
}

Quad2D& Renderer2D::getQuadMemory(unsigned int i) {
	return vertex_buffer_[i];
}

void Renderer2D::freeQuad(unsigned int i) {
	free_quad_list_[i] = false;
}

void Renderer2D::updateBuffers() {
	vbo_->updateBuffer(vertex_buffer_);
}

Renderer2D::~Renderer2D() {
	delete vertex_buffer_;
	delete indices_buffer_;
	delete free_quad_list_;
}

void Renderer2DManager::initialize() {
	geometry_info_.ubbs = nullptr;
	geometry_info_.ubb_count = 0;
	vertex_layout_ = Grindstone::GraphicsAPI::VertexBufferLayout({
		{ Grindstone::GraphicsAPI::VertexFormat::Float2, "vertexPosition", false, Grindstone::GraphicsAPI::AttributeUsage::Position },
		{ Grindstone::GraphicsAPI::VertexFormat::Float2, "vertexColor", false, Grindstone::GraphicsAPI::AttributeUsage::Color },
		{ Grindstone::GraphicsAPI::VertexFormat::Float2, "vertexTexCoord", false, Grindstone::GraphicsAPI::AttributeUsage::TexCoord0 }
	});

	// Fix this soon
	geometry_info_.vertex_layout = &vertex_layout_;
	geometry_info_.vertex_layout_count = 1;
}

Renderer2DManager::~Renderer2DManager() {
}
