#include <iostream>
#include "Core/Engine.hpp"
#include "RenderTerrainSystem.hpp"
#include "TransformSystem.hpp"
#include "../Core/Engine.hpp"
#include "../Utilities/SettingsFile.hpp"
#include <fstream>
#include "Core/Engine.hpp"
#include "AssetManagers/MaterialManager.hpp"
#include "GraphicsWrapper.hpp"
#include "Utilities/SettingsFile.hpp"
#include "Utilities/Logger.hpp"

RenderTerrainComponent::RenderTerrainComponent(GameObjectHandle object_handle, ComponentHandle handle) :
	Component(COMPONENT_RENDER_TERRAIN, object_handle, handle) {}

RenderTerrainSubSystem::RenderTerrainSubSystem(RenderTerrainSystem *system, Space *space) : SubSystem(COMPONENT_RENDER_TERRAIN, space), system_(system) {
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

	VertexBufferCreateInfo vbci;
	vbci.attribute = geometry_info.vads;
	vbci.attributeCount = (uint32_t)geometry_info.vads_count;
	vbci.binding = geometry_info.vbds;
	vbci.bindingCount = geometry_info.vbds_count;
	vbci.content = static_cast<const void *>(vertices.data());
	vbci.count = static_cast<uint32_t>(vertices.size());
	vbci.size = static_cast<uint32_t>(sizeof(glm::vec2) * vertices.size());

	IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	/*if (graphics_wrapper_->SupportsCommandBuffers()) {
		model->vertexBuffer = graphics_wrapper_->CreateVertexBuffer(vbci);
		model->indexBuffer = graphics_wrapper_->CreateIndexBuffer(ibci);
	}
	else {*/

	terrain_drawable_ = new TerrainDrawable();
		VertexArrayObjectCreateInfo vaci;
		vaci.vertexBuffer = terrain_drawable_->vertex_buffer;
		vaci.indexBuffer = terrain_drawable_->index_buffer;
		terrain_drawable_->vertex_array_object = engine.getGraphicsWrapper()->CreateVertexArrayObject(vaci);
		terrain_drawable_->vertex_buffer = engine.getGraphicsWrapper()->CreateVertexBuffer(vbci);
		terrain_drawable_->index_buffer = engine.getGraphicsWrapper()->CreateIndexBuffer(ibci);

		vaci.vertexBuffer = terrain_drawable_->vertex_buffer;
		vaci.indexBuffer = terrain_drawable_->index_buffer;
		terrain_drawable_->vertex_array_object->BindResources(vaci);
		terrain_drawable_->vertex_array_object->Unbind();
		terrain_drawable_->num_indices_ = indices.size();
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

ComponentHandle RenderTerrainSubSystem::addComponent(GameObjectHandle object_handle, rapidjson::Value &params) {
	ComponentHandle component_handle = (ComponentHandle)components_.size();
	components_.emplace_back(object_handle, component_handle);
	auto component = components_.back();

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

			TextureCreateInfo tci;
			tci.data = (unsigned char *)component.heightmap_data_;
			tci.width = tci.height = 512;
			tci.format = FORMAT_COLOR_R16;
			tci.ddscube = false;
			tci.mipmaps = 0;
			tci.options = TextureOptions();
			component.heightmap_ = engine.getGraphicsWrapper()->CreateTexture(tci);

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

	return component_handle;
}

void RenderTerrainSubSystem::removeComponent(ComponentHandle id) {
	components_.erase(components_.begin() + id);
}

RenderTerrainComponent &RenderTerrainSubSystem::getComponent(ComponentHandle id) {
	return components_[id];
}

size_t RenderTerrainSubSystem::getNumComponents() {
	return components_.size();
}

void RenderTerrainSubSystem::writeComponentToJson(ComponentHandle handle, rapidjson::PrettyWriter<rapidjson::StringBuffer> & w) {
}

void RenderTerrainSystem::update(double dt) {
}

UniformBuffer * RenderTerrainSystem::getModelUbo() {
	return model_ubo_;
}

RenderTerrainSystem::RenderTerrainSystem(UniformBufferBinding *ubb) : System(COMPONENT_RENDER_TERRAIN) {
	geometry_info_.vbds_count = 1;
	geometry_info_.vbds = new VertexBindingDescription();
	geometry_info_.vbds->binding = 0;
	geometry_info_.vbds->elementRate = false;
	geometry_info_.vbds->stride = sizeof(glm::vec2);

	geometry_info_.vads_count = 1;
	geometry_info_.vads = new VertexAttributeDescription();
	geometry_info_.vads[0].binding = 0;
	geometry_info_.vads[0].location = 0;
	geometry_info_.vads[0].format = VERTEX_R32_G32;
	geometry_info_.vads[0].size = 2;
	geometry_info_.vads[0].name = "vertexPosition";
	geometry_info_.vads[0].offset = 0;
	geometry_info_.vads[0].usage = ATTRIB_POSITION;

	GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();

	UniformBufferBindingCreateInfo ubbci2;
	ubbci2.binding = 1;
	ubbci2.shaderLocation = "ModelMatrixBuffer";
	ubbci2.size = 128; // sizeof(glm::mat4);
	ubbci2.stages = SHADER_STAGE_VERTEX_BIT;
	model_ubb_ = graphics_wrapper->CreateUniformBufferBinding(ubbci2);

	UniformBufferCreateInfo ubci2;
	ubci2.isDynamic = true;
	ubci2.size = 128;
	ubci2.binding = model_ubb_;
	model_ubo_ = graphics_wrapper->CreateUniformBuffer(ubci2);

	ubbs_ = { ubb, model_ubb_ };

	geometry_info_.ubb_count = ubbs_.size();
	geometry_info_.ubbs = ubbs_.data();
}


void TerrainDrawable::shadowDraw() {
	draw();
}

#include "Core/Scene.hpp"
#include "Core/Space.hpp"

void TerrainDrawable::draw() {
	auto graphics = engine.getGraphicsWrapper();
	bool tess = graphics->SupportsTesselation() && engine.getSettings()->enable_tesselation_;
	/*GrindstoneGeometryType geom = tess ? GEOMETRY_PATCHES : GEOMETRY_TRIANGLE_STRIP;

	engine.graphics->BindTextureBinding(heightmap_texture_binding_);
	for (auto &reference : references) {
		CRender &renderComponent = engine.geometry_system.GetComponent(reference);
		if (renderComponent.should_draw) {
			auto entityID = renderComponent.entity_id;
			auto transform = engine.transformSystem.components[entityID];

			engine.ubo2->UpdateUniformBuffer(&transform.GetModelMatrix());
			engine.ubo->Bind();
			engine.ubo2->Bind();

			engine.graphics->BindVertexArrayObject(vertexArrayObject);

			engine.graphics->DrawImmediateIndexed(geom, true, 0, 0, num_indices_);
		}
	}*/
	auto sp = engine.getScene(0)->spaces_[0];
	RenderTerrainSystem *render_system = (RenderTerrainSystem *)engine.getSystem(COMPONENT_RENDER_TERRAIN);
	RenderTerrainSubSystem *render_subsystem = (RenderTerrainSubSystem *)sp->getSubsystem(COMPONENT_RENDER_TERRAIN);
	TransformSubSystem *transform_system = (TransformSubSystem *)sp->getSubsystem(COMPONENT_TRANSFORM);

	//CRender &renderComponent = engine.geometry_system.GetComponent(reference);
	//if (renderComponent.should_draw) {
	
	auto &component = render_subsystem->getComponent(component_handle_);
	auto game_object_id = component.game_object_handle_;
	auto transform_component_id = sp->getObject(game_object_id);
	auto transform = transform_system->getComponent(game_object_id);

	render_system->getModelUbo()->Bind();
	render_system->getModelUbo()->UpdateUniformBuffer(&transform.model_);

	graphics->BindVertexArrayObject(vertex_array_object);
	graphics->DrawImmediateIndexed(GEOMETRY_TRIANGLE_STRIP, true, 0, 0, num_indices_);
	//}

	/*for (auto &reference : model->references_) {
		RenderStaticMeshComponent &render_component = render_system->getComponent(reference);

		//if (renderComponent.should_draw) {
		auto game_object_id = render_component.game_object_handle_;
		auto transform_component_id = sp->getObject(game_object_id);
		auto transform = transform_system->getComponent(game_object_id);

		engine.getModelManager()->getModelUbo()->Bind();
		engine.getModelManager()->getModelUbo()->UpdateUniformBuffer(&transform.model_);

		engine.getGraphicsWrapper()->BindVertexArrayObject(model->vertex_array_object);
		engine.getGraphicsWrapper()->DrawImmediateIndexed(GEOMETRY_TRIANGLES, true, base_vertex, base_index, num_indices);
		//}
	}*/
}