#include "ModelManager.hpp"
#include "MaterialManager.hpp"
#include "glm/common.hpp"

#include "Core/Utilities.hpp"
#include "Core/Engine.hpp"
#include "../Utilities/Logger.hpp"
#include "../GraphicsCommon/VertexBuffer.hpp"

void MeshStatic::shadowDraw() {
	ModelStatic *model = &engine.getModelManager()->getModel(model_reference);
	for (auto &reference : model->references_) {
		CRender &renderComponent = engine.geometry_system.GetComponent(reference);

		// Cull for shadows seperately
		//if (renderComponent.should_draw) {
		auto entityID = renderComponent.entity_id;
		auto transform = engine.transformSystem.components[entityID];

		engine.ubo2->UpdateUniformBuffer(&transform.GetModelMatrix());
		engine.ubo2->Bind();

		engine.getGraphicsWrapper()->BindVertexArrayObject(model->vertex_array_object);
		engine.getGraphicsWrapper()->DrawImmediateIndexed(GEOMETRY_TRIANGLES, true, base_vertex, base_index, num_indices);
		//}
	}
}

void MeshStatic::draw() {
	ModelStatic *model = &engine.getModelManager()->getModel(model_reference);
	for (auto &reference : model->references_) {
		CRender &renderComponent = engine.geometry_system.GetComponent(reference);

		// Cull for shadows seperately
		//if (renderComponent.should_draw) {
		auto entityID = renderComponent.entity_id;
		auto transform = engine.transformSystem.components[entityID];

		engine.ubo2->UpdateUniformBuffer(&transform.GetModelMatrix());
		engine.ubo2->Bind();

		engine.getGraphicsWrapper()->BindVertexArrayObject(model->vertex_array_object);
		engine.getGraphicsWrapper()->DrawImmediateIndexed(GEOMETRY_TRIANGLES, true, base_vertex, base_index, num_indices);
		//}
	}
}

ModelManager::ModelManager() {
	geometry_info_.vbds_count = 1;
	geometry_info_.vbds = new VertexBindingDescription();
	geometry_info_.vbds->binding = 0;
	geometry_info_.vbds->elementRate = false;
	geometry_info_.vbds->stride = sizeof(Vertex);

	geometry_info_.vads_count = 4;
	geometry_info_.vads = new VertexAttributeDescription[4];
	geometry_info_.vads[0].binding = 0;
	geometry_info_.vads[0].location = 0;
	geometry_info_.vads[0].format = VERTEX_R32_G32_B32;
	geometry_info_.vads[0].size = 3;
	geometry_info_.vads[0].name = "vertexPosition";
	geometry_info_.vads[0].offset = offsetof(Vertex, positions);
	geometry_info_.vads[0].usage = ATTRIB_POSITION;

	geometry_info_.vads[1].binding = 0;
	geometry_info_.vads[1].location = 1;
	geometry_info_.vads[1].format = VERTEX_R32_G32_B32;
	geometry_info_.vads[1].size = 3;
	geometry_info_.vads[1].name = "vertexNormal";
	geometry_info_.vads[1].offset = offsetof(Vertex, normal);
	geometry_info_.vads[1].usage = ATTRIB_NORMAL;

	geometry_info_.vads[2].binding = 0;
	geometry_info_.vads[2].location = 2;
	geometry_info_.vads[2].format = VERTEX_R32_G32_B32;
	geometry_info_.vads[2].size = 3;
	geometry_info_.vads[2].name = "vertexTangent";
	geometry_info_.vads[2].offset = offsetof(Vertex, tangent);
	geometry_info_.vads[2].usage = ATTRIB_TANGENT;

	geometry_info_.vads[3].binding = 0;
	geometry_info_.vads[3].location = 3;
	geometry_info_.vads[3].format = VERTEX_R32_G32;
	geometry_info_.vads[3].size = 2;
	geometry_info_.vads[3].name = "vertexTexCoord";
	geometry_info_.vads[3].offset = offsetof(Vertex, texCoord);
	geometry_info_.vads[3].usage = ATTRIB_TEXCOORD0;
}

void ModelManager::preloadModel(std::string path) {
	// Check for previous model
	if (models_map_.find(path) != models_map_.end()) {
		models_map_[path];
	}

	// Insert new model
	ModelReference ref = ModelReference(models_.size());
	models_.emplace_back();

	// Prepare it for loading
	unloaded_.emplace_back(ref);
}

void ModelManager::loadModel(std::string path) {
	// Check for previous model
	if (models_map_.find(path) != models_map_.end()) {
		models_map_[path];
	}

	// Insert new model
	ModelReference ref = ModelReference(models_.size());
	models_.emplace_back();

	// Load it
	loadModel(models_.back());
}

void ModelManager::loadPreloaded() {
	for (auto model : unloaded_) {
		loadModel(models_[model]);
	}
}

ModelStatic &ModelManager::getModel(ModelReference reference) {
	return models_[reference];
}

void ModelManager::loadModel(ModelStatic &model) {
	GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	MaterialManager *material_system = engine.getMaterialManager();

	// Load File
	std::string path = std::string(model.path_);
	std::ifstream input(path, std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		LOG_ERROR("Failed to open file: %s!\n", path);
		return;
	}

	LOG("Model reading from: %s!\n", path);

	// Read size and create buffer
	size_t fileSize = (size_t)input.tellg();
	std::vector<char> buffer(fileSize);

	// Read file to buffer
	input.seekg(0);
	input.read(buffer.data(), fileSize);
	input.close();

	// Get Header
	ModelFormatHeader inFormat;
	void *offset = buffer.data();
	memcpy(&inFormat, offset, sizeof(ModelFormatHeader));
	offset = static_cast<char*>(offset) + sizeof(ModelFormatHeader);
	
	// Get Submesh Data
	model.meshes.resize(inFormat.num_meshes);
	uint32_t size = inFormat.num_meshes * sizeof(MeshCreateInfo);
	std::vector<MeshCreateInfo> temp_meshes(inFormat.num_meshes);
	memcpy(temp_meshes.data(), offset, size);
	offset = static_cast<char*>(offset) + size;

	// Get Vertex Data
	std::vector<Vertex> vertices;
	vertices.resize(inFormat.num_vertices);
	size = uint32_t(inFormat.num_vertices * sizeof(Vertex));
	memcpy(&vertices[0], offset, size);
	offset = static_cast<char*>(offset) + size;

	// Get Index Data
	std::vector<unsigned int> indices;
	indices.resize(inFormat.num_indices);
	size = uint32_t(inFormat.num_indices * sizeof(unsigned int));
	memcpy(&indices[0], offset, size);
	offset = static_cast<char*>(offset) + size;

	// Load Materials
	LOG("Loading %s materials.\n", inFormat.num_materials);
	std::vector<MaterialReference> material_references;
	material_references.resize(inFormat.num_materials);

	std::vector<Material *> materials;
	materials.resize(inFormat.num_materials);
	char *words = (char *)offset;
	for (unsigned int i = 0; i < inFormat.num_materials; i++) {
		std::string material_name = std::string(words);
		material_references[i] = material_system->loadMaterial(geometry_info_, material_name);
		words = strchr(words, '\0') + 1;
	}

	// Create Vertex Buffer
	VertexBufferCreateInfo vbci;
	vbci.attribute = geometry_info_.vads;
	vbci.attributeCount = (uint32_t)geometry_info_.vads_count;
	vbci.binding = geometry_info_.vbds;
	vbci.bindingCount = geometry_info_.vbds_count;
	vbci.content = static_cast<const void *>(vertices.data());
	vbci.count = static_cast<uint32_t>(vertices.size());
	vbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());

	// Create Index Buffer
	IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	// Create Vertex Array Object / CommandBuffer
	VertexArrayObjectCreateInfo vaci;
	vaci.vertexBuffer = model.vertex_buffer;
	vaci.indexBuffer = model.index_buffer;
	model.vertex_array_object = graphics_wrapper->CreateVertexArrayObject(vaci);
	vaci.vertexBuffer = model.vertex_buffer = graphics_wrapper->CreateVertexBuffer(vbci);
	vaci.indexBuffer = model.index_buffer = graphics_wrapper->CreateIndexBuffer(ibci);
	model.vertex_array_object->BindResources(vaci);
	model.vertex_array_object->Unbind();

	// Create Final Submeshes
	for (unsigned int i = 0; i < inFormat.num_meshes; i++) {
		// Use the temporarily uint32_t material as an ID for the actual material.
		MeshCreateInfo &temp_mesh = temp_meshes[i];
		MeshStatic &current_mesh = model.meshes[i];
		current_mesh.model_reference = model.id;
		current_mesh.material_reference = material_references[temp_mesh.material_index];
		current_mesh.num_indices = temp_mesh.num_indices;
		current_mesh.base_vertex = temp_mesh.base_vertex;
		current_mesh.base_index = temp_mesh.base_index;
		auto mat = engine.getMaterialManager()->getMaterial(current_mesh.material_reference);
		mat->m_meshes.push_back(reinterpret_cast<MeshStatic *>(&current_mesh));
	}
}