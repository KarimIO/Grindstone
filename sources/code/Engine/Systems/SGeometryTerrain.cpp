#include "SGeometryTerrain.hpp"
#include "Core/Engine.hpp"
#include <stb/stb_image.h>

SGeometryTerrain::SGeometryTerrain(MaterialManager *material_system, GraphicsWrapper * graphics_wrapper, std::vector<UniformBufferBinding *> ubbs) : graphics_wrapper_(graphics_wrapper), material_system_(material_system) {
	geometry_info_.ubb_count = ubbs.size();
	geometry_info_.ubbs = new UniformBufferBinding *[ubbs.size()];

	for (unsigned int i = 0; i < geometry_info_.ubb_count; i++) {
		geometry_info_.ubbs[i] = ubbs[i];
	}

	geometry_info_.vbds_count = 1;
	geometry_info_.vbds = new VertexBindingDescription();
	geometry_info_.vbds->binding = 0;
	geometry_info_.vbds->elementRate = false;
	geometry_info_.vbds->stride = sizeof(glm::vec2);

	geometry_info_.vads_count = 1;
	geometry_info_.vads = new VertexAttributeDescription();
	geometry_info_.vads->binding = 0;
	geometry_info_.vads->location = 0;
	geometry_info_.vads->format = VERTEX_R32_G32;
	geometry_info_.vads->size = 2;
	geometry_info_.vads->name = "vertexPosition";
	geometry_info_.vads->offset = 0;
	geometry_info_.vads->usage = ATTRIB_POSITION;
}

void SGeometryTerrain::LoadModel(CTerrain * model) {
	int texWidth, texHeight, texChannels;
	unsigned char *data = stbi_load(model->getHeightmap().c_str(), &texWidth, &texHeight, &texChannels, 4);
	if (!data) {
		printf("Texture failed to load!: %s \n", model->getHeightmap().c_str());
		return;
	}

	TextureCreateInfo createInfo;
	createInfo.data = data;
	createInfo.mipmaps = 0;
	createInfo.format = FORMAT_COLOR_R8G8B8A8;
	createInfo.width = texWidth;
	createInfo.height = texHeight;
	createInfo.ddscube = false;

	model->heightmap_texture_ = graphics_wrapper_->CreateTexture(createInfo);

	std::vector<TextureSubBinding> heightmapBindings;
	heightmapBindings.reserve(1);
	heightmapBindings.emplace_back("heightmap", 0);

	TextureBindingLayoutCreateInfo tblci;
	tblci.bindingLocation = 0;
	tblci.bindings = heightmapBindings.data();
	tblci.bindingCount = (uint32_t)heightmapBindings.size();
	tblci.stages = SHADER_STAGE_VERTEX_BIT;
	TextureBindingLayout *heightmapLayout = graphics_wrapper_->CreateTextureBindingLayout(tblci);

	SingleTextureBind stb;
	stb.texture = model->heightmap_texture_;
	stb.address = 0;

	TextureBindingCreateInfo ci;
	ci.textures = &stb;
	ci.textureCount = 1;
	ci.layout = heightmapLayout;
	model->heightmap_texture_binding_ = graphics_wrapper_->CreateTextureBinding(ci);

	std::vector<glm::vec2> vertices;
	std::vector<unsigned int> indices;

	int tile_w = 32, tile_h = 32;
	float w_factor = 1.0f / tile_w;
	float h_factor = 1.0f / tile_h;

	vertices.reserve((tile_w + 1) * (tile_h + 1));
	for (int i = 0; i < tile_w; i++) {
		for (int j = 0; j < tile_h; j++) {
			vertices.push_back(glm::vec2(float(i) * w_factor, float(j) * h_factor));
		}
	}

	
	//UINT* indices = new UINT[arr_size];
	if (engine.graphics_wrapper_->SupportsTesselation() && engine.settings.enableTesselation) {
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
	else {
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
	}

	if (graphics_wrapper_->SupportsTesselation() && engine.settings.enableTesselation)
		model->material = material_system_->GetMaterial(material_system_->CreateMaterial(geometry_info_, "../assets/materials/terrain.gmat"));
	else
		model->material = material_system_->GetMaterial(material_system_->CreateMaterial(geometry_info_, "../assets/materials/terrain_notess.gmat"));

	model->material->m_meshes.push_back(reinterpret_cast<Mesh *>(model));

	model->num_indices_ = indices.size();

	VertexBufferCreateInfo vbci;
	vbci.attribute = geometry_info_.vads;
	vbci.attributeCount = (uint32_t)geometry_info_.vads_count;
	vbci.binding = geometry_info_.vbds;
	vbci.bindingCount = geometry_info_.vbds_count;
	vbci.content = static_cast<const void *>(vertices.data());
	vbci.count = static_cast<uint32_t>(vertices.size());
	vbci.size = static_cast<uint32_t>(sizeof(glm::vec2) * vertices.size());

	IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	if (graphics_wrapper_->SupportsCommandBuffers()) {
		model->vertexBuffer = graphics_wrapper_->CreateVertexBuffer(vbci);
		model->indexBuffer = graphics_wrapper_->CreateIndexBuffer(ibci);
	}
	else {
		VertexArrayObjectCreateInfo vaci;
		vaci.vertexBuffer = model->vertexBuffer;
		vaci.indexBuffer = model->indexBuffer;
		model->vertexArrayObject = graphics_wrapper_->CreateVertexArrayObject(vaci);
		model->vertexBuffer = graphics_wrapper_->CreateVertexBuffer(vbci);
		model->indexBuffer = graphics_wrapper_->CreateIndexBuffer(ibci);

		vaci.vertexBuffer = model->vertexBuffer;
		vaci.indexBuffer = model->indexBuffer;
		model->vertexArrayObject->BindResources(vaci);
		model->vertexArrayObject->Unbind();
	}
}

void SGeometryTerrain::LoadGeometry(unsigned int render_id, std::string path) {
	for (size_t i = 0; i < models.size(); i++) {
		if (models[i].getHeightmap() == path) {
			models[i].references.push_back((uint32_t)render_id);
			return;
		}
	}

	unloadedModelIDs.push_back((unsigned int)(models.size()));
	models.push_back({ path,{ render_id } });
}

void SGeometryTerrain::LoadPreloaded() {
	for (unsigned int unloadedModelID : unloadedModelIDs) {
		CTerrain *model = &models[unloadedModelID];
		LoadModel(model);
	}
}

void SGeometryTerrain::Cull(CCamera * cam) {
	for (auto &model : models) {
		for (auto &reference_id : model.references) {
			auto &reference = engine.geometry_system.GetComponent(reference_id);
			reference.should_draw = true;
			if (reference.should_draw) {
				model.material->IncrementDrawCount();
			}
		}
	}
}

std::string CTerrain::getHeightmap() {
	return heightmap_dir_;
}

void CTerrain::setHeightmap(std::string dir) {
	heightmap_dir_ = dir;
}

void CTerrain::ShadowDraw()
{
}

void CTerrain::Draw() {
	bool tess = engine.graphics_wrapper_->SupportsTesselation() && engine.settings.enableTesselation;
	GrindstoneGeometryType geom = tess ? GEOMETRY_PATCHES : GEOMETRY_TRIANGLE_STRIP;

	engine.graphics_wrapper_->BindTextureBinding(heightmap_texture_binding_);
	for (auto &reference : references) {
		CRender &renderComponent = engine.geometry_system.GetComponent(reference);
		if (renderComponent.should_draw) {
			auto entityID = renderComponent.entity_id;
			auto transform = engine.transformSystem.components[entityID];

			engine.ubo2->UpdateUniformBuffer(&transform.GetModelMatrix());
			engine.ubo->Bind();
			engine.ubo2->Bind();

			engine.graphics_wrapper_->BindVertexArrayObject(vertexArrayObject);
			
			engine.graphics_wrapper_->DrawImmediateIndexed(geom, true, 0, 0, num_indices_);
		}
	}
}

void CTerrain::DrawDeferred(CommandBuffer *) {
}
