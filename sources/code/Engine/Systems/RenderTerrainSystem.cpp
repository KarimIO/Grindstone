#include <iostream>
#include "Core/Engine.hpp"
#include "RenderTerrainSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Utilities/SettingsFile.hpp"
#include <fstream>
#include "Core/Engine.hpp"
#include "AssetManagers/MaterialManager.hpp"
#include <GraphicsCommon/GraphicsWrapper.hpp>
#include "Utilities/SettingsFile.hpp"

RenderTerrainComponent::RenderTerrainComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_RENDER_TERRAIN, object_handle, handle) {}

RenderTerrainSubSystem::RenderTerrainSubSystem(Space *space) : SubSystem(COMPONENT_RENDER_TERRAIN, space), system_((RenderTerrainSystem*)engine.getSystem(COMPONENT_RENDER_TERRAIN)) {
}

void RenderTerrainComponent::generateMesh() {
	std::vector<glm::vec2> vertices;
	std::vector<unsigned int> indices;

	int tile_w = 128, tile_h = 128;
	float w_factor = 1.0f / tile_w;
	float h_factor = 1.0f / tile_h;

	vertices.reserve((tile_w + 1) * (tile_h + 1));
	for (int i = 0; i < tile_w; i++) {
		for (int j = 0; j < tile_h; j++) {
			vertices.push_back(glm::vec2(float(i) * w_factor, float(j) * h_factor));
		}
	}


	//UINT* indices = new UINT[arr_size];
	/*if (engine.getGraphicsWrapper()->SupportsTesselation() && engine.getSettings().enableTesselation) {
		int array_size = (tile_w - 1) * (tile_h - 1) * 6;
		indices.resize(array_size);
		int i = 0;
		for (int y = 0; y < tile_h - 1; ++y) {
			for (int x = 0; x < tile_w - 1; ++x) {
				indices[i++] = x + y * tile_w;
				indices[i++] = x + 1 + y * tile_w;
				indices[i++] = x + (y + 1) * tile_w;

				indices[i++] = x + 1 + y * tile_w;
				indices[i++] = x + 1 + (y + 1) * tile_w;
				indices[i++] = x + (y + 1) * tile_w;
			}
		}
	}
	else {*/
		int strip_size = tile_w * 2;
		int num_strips = tile_h - 1;
		int array_size = strip_size * num_strips + (num_strips - 1) * 4; // degenerate triangles
		indices.resize(array_size);

		int i = 0;
		for (int s = 0; s < num_strips; ++s) {
			int m = 0;
			for (int n = 0; n < tile_w; ++n) {
				m = n + s * tile_w;
				indices[i++] = m + tile_w;
				indices[i++] = m;
			}
			if (s < num_strips - 1) { // create indices for degenerate triangles to get us back to the start.
				indices[i++] = m;
				indices[i++] = m - tile_w + 1;
				indices[i++] = m - tile_w + 1;
				indices[i++] = m - tile_w + 1;
			}
		}
	//}

	auto &geometry_info = ((RenderTerrainSystem *)engine.getSystem(COMPONENT_RENDER_TERRAIN))->geometry_info_;

	Grindstone::GraphicsAPI::VertexBufferCreateInfo vbci;
	vbci.layout = geometry_info.vertex_layout;
	vbci.content = static_cast<const void *>(vertices.data());
	vbci.count = static_cast<uint32_t>(vertices.size());
	vbci.size = static_cast<uint32_t>(sizeof(glm::vec2) * vertices.size());

	Grindstone::GraphicsAPI::IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	/*if (graphics_wrapper_->SupportsCommandBuffers()) {
		model->Grindstone::GraphicsAPI::VertexBuffer = graphics_wrapper_->createVertexBuffer(vbci);
		model->indexBuffer = graphics_wrapper_->createIndexBuffer(ibci);
	}
	else {*/

		terrain_drawable_ = new TerrainDrawable();
		terrain_drawable_->vertex_buffer = engine.getGraphicsWrapper()->createVertexBuffer(vbci);
		terrain_drawable_->index_buffer = engine.getGraphicsWrapper()->createIndexBuffer(ibci);

		Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo vaci;
		vaci.vertex_buffers = &terrain_drawable_->vertex_buffer;
		vaci.vertex_buffer_count = 1;
		vaci.index_buffer = terrain_drawable_->index_buffer;
		terrain_drawable_->vertex_array_object = engine.getGraphicsWrapper()->createVertexArrayObject(vaci);


		terrain_drawable_->num_indices_ = (unsigned int)indices.size();
	//}

	auto matmgr = engine.getMaterialManager();
	auto mat = matmgr->getMaterial(material_);
	mat->m_meshes.push_back(terrain_drawable_);
}

ComponentHandle RenderTerrainSubSystem::addComponent(GameObjectHandle object_handle) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto &component = components_.back();

	return component_handle;
}

RenderTerrainSubSystem::~RenderTerrainSubSystem() {
}

#if 0
void RenderTerrainSubSystem::setComponent(ComponentHandle component_handle, rapidjson::Value & params) {
	auto &component = components_[component_handle];

	if (params.HasMember("path")) {
		auto path = params["path"].GetString();
		component.path_ = std::string("../assets/") + path;
		// If Raw file
		std::ifstream input(component.path_, std::ios::ate | std::ios::binary);
		if (!input.is_open()) {
			GRIND_ERROR("Invalid path to terrain.");
		} 
		else {
			/*component.heightmap_data_ = 0;
			
			component.heightmap_size_ = input.tellg();
			component.heightmap_data_ = new char[component.heightmap_size_];

			input.seekg(0);
			input.read(component.heightmap_data_, component.heightmap_size_);

			Grindstone::GraphicsAPI::TextureCreateInfo tci;
			tci.data = (unsigned char *)component.heightmap_data_;
			tci.width = tci.height = 512;
			tci.format = Grindstone::GraphicsAPI::ColorFormat::R16;
			tci.ddscube = false;
			tci.mipmaps = 0;
			tci.options = TextureOptions();
			component.heightmap_ = engine.getGraphicsWrapper()->createTexture(tci);

			input.close();*/
		}
		// If other kind of image
	}

	if (params.HasMember("materialPath")) {
		auto path = params["materialPath"].GetString();
		component.material_path_ = path;
	}
	else {
		component.material_path_ = "../assets/materials/terrain_notess.gmat";
	}

	component.material_ = engine.getMaterialManager()->loadMaterial(system_->geometry_info_, component.material_path_);
	Material *mat = engine.getMaterialManager()->getMaterial(component.material_);

	component.generateMesh();
}
#endif

void RenderTerrainSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

RenderTerrainComponent &RenderTerrainSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

Component * RenderTerrainSubSystem::getBaseComponent(ComponentHandle component_handle) {
	return &components_[component_handle];
}

size_t RenderTerrainSubSystem::getNumComponents() {
	return components_.size();
}

void RenderTerrainSystem::update() {
}

Grindstone::GraphicsAPI::UniformBuffer * RenderTerrainSystem::getModelUbo() {
	return model_ubo_;
}

RenderTerrainSystem::RenderTerrainSystem(Grindstone::GraphicsAPI::UniformBufferBinding *ubb) : System(COMPONENT_RENDER_TERRAIN) {
	GRIND_PROFILE_FUNC();

	vertex_layout_ = Grindstone::GraphicsAPI::VertexBufferLayout({
		{ Grindstone::GraphicsAPI::VertexFormat::Float2, "vertexPosition", false, Grindstone::GraphicsAPI::AttributeUsage::Position }
	});

	geometry_info_.vertex_layout = &vertex_layout_;
	geometry_info_.vertex_layout_count = 1;

	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();

	Grindstone::GraphicsAPI::UniformBufferBindingCreateInfo ubbci2;
	ubbci2.binding = 1;
	ubbci2.shaderLocation = "ModelMatrixBuffer";
	ubbci2.size = 128; // sizeof(glm::mat4);
	ubbci2.stages = Grindstone::GraphicsAPI::ShaderStageBit::Vertex;
	model_ubb_ = graphics_wrapper->createUniformBufferBinding(ubbci2);

	Grindstone::GraphicsAPI::UniformBufferCreateInfo ubci2;
	ubci2.isDynamic = true;
	ubci2.size = 128;
	ubci2.binding = model_ubb_;
	model_ubo_ = graphics_wrapper->createUniformBuffer(ubci2);

	ubbs_ = { ubb, model_ubb_ };

	geometry_info_.ubb_count = (unsigned int)ubbs_.size();
	geometry_info_.ubbs = ubbs_.data();
}


void TerrainDrawable::shadowDraw() {
	draw();
}

#include "Core/Scene.hpp"
#include "Core/Space.hpp"

void TerrainDrawable::draw() {
	auto graphics = engine.getGraphicsWrapper();
	bool tess = graphics->supportsTesselation() && engine.getSettings()->enable_tesselation_;
	/*GrindstoneGeometryType geom = tess ? GEOMETRY_PATCHES : Grindstone::GraphicsAPI::GeometryType::TriangleStrips;

	engine.graphics->bindTextureBinding(heightmap_texture_binding_);
	for (auto &reference : references) {
		CRender &renderComponent = engine.geometry_system.GetComponent(reference);
		if (renderComponent.should_draw) {
			auto entityID = renderComponent.entity_id;
			auto transform = engine.transformSystem.components[entityID];

			engine.ubo2->updateBuffer(&transform.GetModelMatrix());
			engine.ubo->Bind();
			engine.ubo2->Bind();

			engine.graphics->bindVertexArrayObject(Grindstone::GraphicsAPI::VertexArrayObject);

			engine.graphics->drawImmediateIndexed(geom, true, 0, 0, num_indices_);
		}
	}*/
	auto sp = engine.getSpace(0);
	RenderTerrainSystem *render_system = (RenderTerrainSystem *)engine.getSystem(COMPONENT_RENDER_TERRAIN);
	RenderTerrainSubSystem *render_subsystem = (RenderTerrainSubSystem *)sp->getSubsystem(COMPONENT_RENDER_TERRAIN);
	TransformSubSystem *transform_system = (TransformSubSystem *)sp->getSubsystem(COMPONENT_TRANSFORM);

	//CRender &renderComponent = engine.geometry_system.GetComponent(reference);
	//if (renderComponent.should_draw) {
	
	auto &component = render_subsystem->getComponent(component_handle_);
	auto game_object_id = component.game_object_handle_;
	auto transform_component_id = sp->getObject(game_object_id);
	auto transform = transform_system->getComponent(game_object_id);

	render_system->getModelUbo()->bind();
	render_system->getModelUbo()->updateBuffer(&transform.model_);
	
	graphics->bindVertexArrayObject(vertex_array_object);
	graphics->drawImmediateIndexed(Grindstone::GraphicsAPI::GeometryType::TriangleStrips, true, 0, 0, num_indices_);
	//}

	/*for (auto &reference : model->references_) {
		RenderStaticMeshComponent &render_component = render_system->getComponent(reference);

		//if (renderComponent.should_draw) {
		auto game_object_id = render_component.game_object_handle_;
		auto transform_component_id = sp->getObject(game_object_id);
		auto transform = transform_system->getComponent(game_object_id);

		engine.getModelManager()->getModelUbo()->Bind();
		engine.getModelManager()->getModelUbo()->updateBuffer(&transform.model_);

		engine.getGraphicsWrapper()->bindVertexArrayObject(model->vertex_array_object);
		engine.getGraphicsWrapper()->drawImmediateIndexed(Grindstone::GraphicsAPI::GeometryType::TriangleStrips, true, base_vertex, base_index, num_indices);
		//}
	}*/
}