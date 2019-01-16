#include "ModelManager.hpp"
#include "MaterialManager.hpp"
#include "../Systems/RenderStaticMeshSystem.hpp"
#include "../Systems/TransformSystem.hpp"
#include "glm/common.hpp"

#include "Core/Utilities.hpp"
#include "Core/Engine.hpp"
#include "../Utilities/Logger.hpp"
#include "../GraphicsCommon/VertexBuffer.hpp"
#include "GraphicsWrapper.hpp"

#include  "Core/Scene.hpp"
#include  "Core/Space.hpp"

ModelStatic::ModelStatic() {}
ModelStatic::ModelStatic(ModelReference handle, std::string path, ComponentHandle ref) : handle_(handle), path_(path) {
	references_.push_back(ref);
};

void MeshStatic::shadowDraw() {
	draw();
}

void MeshStatic::draw() {
	auto sp = engine.getScene(0)->spaces_[0]; 
	RenderStaticMeshSubSystem *render_system = (RenderStaticMeshSubSystem *)sp->getSubsystem(COMPONENT_RENDER_STATIC_MESH);
	TransformSubSystem *transform_system = (TransformSubSystem *)sp->getSubsystem(COMPONENT_TRANSFORM);

	ModelStatic *model = &engine.getModelManager()->getModel(model_reference);
	for (auto &reference : model->references_) {
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
	}
}

ModelManager::ModelManager(UniformBufferBinding *ubb) {
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

ModelReference ModelManager::preloadModel(ComponentHandle render_handle, std::string path) {
	// Check for previous model
	if (models_map_.find(path) != models_map_.end()) {
		models_map_[path];
	}

	// Insert new model
	ModelReference ref = ModelReference(models_.size());
	models_.emplace_back(ref, path, render_handle);

	// Prepare it for loading
	unloaded_.emplace_back(ref);

	return ref;
}

ModelReference ModelManager::loadModel(ComponentHandle render_handle, std::string path) {
	// Check for previous model
	if (models_map_.find(path) != models_map_.end()) {
		models_map_[path];
	}

	// Insert new model
	ModelReference ref = ModelReference(models_.size());
	models_.emplace_back(ref, path, render_handle);

	// Load it
	loadModel(models_.back());

	return ref;
}

void ModelManager::loadPreloaded() {
	for (auto model : unloaded_) {
		loadModel(models_[model]);
	}
}

ModelStatic &ModelManager::getModel(ModelReference reference) {
	return models_[reference];
}

UniformBuffer * ModelManager::getModelUbo() {
	return model_ubo_;
}

void ModelManager::loadModel(ModelStatic &model) {
	GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	MaterialManager *material_system = engine.getMaterialManager();

	std::string path = std::string("../assets/" + model.path_);
	std::ifstream input(path, std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		std::cerr << "Failed to open file: " << path << "!\n";
		return;
	}

	std::cout << "Model reading from: " << path << "!\n";

	size_t fileSize = (size_t)input.tellg();
	std::vector<char> buffer(fileSize);

	input.seekg(0);
	input.read(buffer.data(), fileSize);

	ModelFormatHeader inFormat;
	void *offset = buffer.data();
	memcpy(&inFormat, offset, sizeof(ModelFormatHeader));
	offset = static_cast<char*>(offset) + sizeof(ModelFormatHeader);
	switch (inFormat.bounding_type) {
	case BOUNDING_SPHERE:
		model.bounding = new BoundingSphere();
		break;
	case BOUNDING_BOX:
		model.bounding = new BoundingBox();
		break;
	}
	memcpy(model.bounding->GetData(), offset, model.bounding->GetSize());
	model.bounding->Print();

	offset = static_cast<char*>(offset) + model.bounding->GetSize();

	std::vector<Vertex> vertices;
	std::vector<VertexWeights> vertex_weights;
	std::vector<unsigned int> indices;

	std::vector<MeshCreateInfo> temp_meshes(inFormat.num_meshes);
	model.meshes.resize(inFormat.num_meshes);
	vertices.resize(inFormat.num_vertices);
	vertex_weights.resize(inFormat.num_vertices);
	indices.resize(inFormat.num_indices);

	// Copy Meshes
	uint32_t size = inFormat.num_meshes * sizeof(MeshCreateInfo);
	memcpy(temp_meshes.data(), offset, size);
	offset = static_cast<char*>(offset) + size;

	// Copy Vertices
	size = inFormat.num_vertices * sizeof(Vertex);
	memcpy(vertices.data(), offset, size);
	offset = static_cast<char*>(offset) + size;

	// Copy Vertex Weights
	if (inFormat.has_bones) {
		size = inFormat.num_vertices * sizeof(VertexWeights);
		memcpy(vertex_weights.data(), offset, size);
		offset = static_cast<char*>(offset) + size;
	}

	// Copy Indices
	size = inFormat.num_indices * sizeof(unsigned int);
	memcpy(indices.data(), offset, size);
	offset = static_cast<char*>(offset) + size;

	std::cout << "Loading " << inFormat.num_materials << " materials.\n";
	std::vector<MaterialReference> materialReferences;
	materialReferences.resize(inFormat.num_materials);

	std::vector<Material *> materials;
	materials.resize(inFormat.num_materials);
	char *words = (char *)offset;
	for (unsigned int i = 0; i < inFormat.num_materials; i++) {
		// Need to add a non-lazyload material
		materialReferences[i] = material_system->loadMaterial(geometry_info_, words);
		words = strchr(words, '\0') + 1;
	}

	for (unsigned int i = 0; i < inFormat.num_materials; i++) {
		materials[i] = material_system->getMaterial(materialReferences[i]);
	}

	input.close();

	VertexBufferCreateInfo vbci;
	vbci.attribute = geometry_info_.vads;
	vbci.attributeCount = (uint32_t)geometry_info_.vads_count;
	vbci.binding = geometry_info_.vbds;
	vbci.bindingCount = geometry_info_.vbds_count;
	vbci.content = static_cast<const void *>(vertices.data());
	vbci.count = static_cast<uint32_t>(vertices.size());
	vbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());

	/*VertexBufferCreateInfo svbci;
	svbci.attribute = geometry_info_.vads;
	svbci.attributeCount = (uint32_t)geometry_info_.vads_count;
	svbci.binding = geometry_info_.vbds;
	svbci.bindingCount = geometry_info_.vbds_count;
	svbci.content = static_cast<const void *>(vertices.data());
	svbci.count = static_cast<uint32_t>(vertices.size());
	svbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());*/

	IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	if (graphics_wrapper->SupportsCommandBuffers()) {
		model.vertex_buffer = graphics_wrapper->CreateVertexBuffer(vbci);
		model.index_buffer = graphics_wrapper->CreateIndexBuffer(ibci);
	}
	else {
		VertexArrayObjectCreateInfo vaci;
		vaci.vertexBuffer = model.vertex_buffer;
		vaci.indexBuffer = model.index_buffer;
		model.vertex_array_object = graphics_wrapper->CreateVertexArrayObject(vaci);
		model.vertex_buffer = graphics_wrapper->CreateVertexBuffer(vbci);
		model.index_buffer = graphics_wrapper->CreateIndexBuffer(ibci);

		vaci.vertexBuffer = model.vertex_buffer;
		vaci.indexBuffer = model.index_buffer;
		model.vertex_array_object->BindResources(vaci);
		model.vertex_array_object->Unbind();


		/*VertexArrayObjectCreateInfo vaci;
		vaci.vertexBuffer = model.shadowVertexBuffer;
		vaci.indexBuffer = model.indexBuffer;
		model.shadowVertexArrayObject = graphics_wrapper_->CreateVertexArrayObject(vaci);
		model.shadowVertexBuffer = graphics_wrapper_->CreateVertexBuffer(svbci);
		model.indexBuffer = graphics_wrapper_->CreateIndexBuffer(ibci);

		vaci.vertexBuffer = model.shadowVertexBuffer;
		vaci.indexBuffer = model.indexBuffer;
		model.shadowVertexArrayObject->BindResources(vaci);
		model.shadowVertexArrayObject->Unbind();*/

	}

	for (unsigned int i = 0; i < inFormat.num_meshes; i++) {
		// Use the temporarily uint32_t material as an ID for the actual material.
		MeshCreateInfo &temp_mesh = temp_meshes[i];
		MeshStatic &current_mesh = model.meshes[i];
		current_mesh.model_reference = model.handle_;
		//current_mesh.material_reference = ;
		current_mesh.num_indices = temp_mesh.num_indices;
		current_mesh.base_vertex = temp_mesh.base_vertex;
		current_mesh.base_index = temp_mesh.base_index;
		//current_mesh.material->m_meshes.push_back(reinterpret_cast<MeshStatic *>(&current_mesh));
		materials[temp_mesh.material_index]->m_meshes.push_back(&current_mesh);
	}
	/*

	// Load File
	std::string path = std::string("../assets/" + model.path_);
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
	auto a = (char *)offset - buffer.data();
	std::cout << "Header:" << a << "\n";
	
	// Get Submesh Data
	model.meshes.resize(inFormat.num_meshes);
	uint32_t size = inFormat.num_meshes * sizeof(MeshCreateInfo);
	std::vector<MeshCreateInfo> temp_meshes(inFormat.num_meshes);
	memcpy(temp_meshes.data(), offset, size);
	offset = static_cast<char*>(offset) + size;
	a = (char *)offset - buffer.data();
	std::cout << "Submesh:" << a << "\n";

	// Get Vertex Data
	std::vector<Vertex> vertices;
	vertices.resize(inFormat.num_vertices);
	size = uint32_t(inFormat.num_vertices * sizeof(Vertex));
	memcpy(&vertices[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	a = (char *)offset - buffer.data();
	std::cout << "Vertex:" << a << "\n";

	// Get Index Data
	std::vector<unsigned int> indices;
	indices.resize(inFormat.num_indices);
	size = uint32_t(inFormat.num_indices * sizeof(unsigned int));
	memcpy(&indices[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	a = (char *)offset - buffer.data();
	std::cout << "Index:" << a << "\n";

	// Load Materials
	printf("Loading %u materials.\n", inFormat.num_materials);
	std::vector<MaterialReference> material_references;
	material_references.resize(inFormat.num_materials);

	std::vector<Material *> materials;
	materials.resize(inFormat.num_materials);
	char *words = (char *)offset;
	a = words - buffer.data();
	std::cout << "Material:" << a << "\n";
	for (unsigned int i = 0; i < inFormat.num_materials; i++) {
		std::string material_name = std::string(words);
		std::cout << material_name << "\n";
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
	model.vertex_array_object = engine.getGraphicsWrapper()->CreateVertexArrayObject(vaci);
	vaci.vertexBuffer = model.vertex_buffer = engine.getGraphicsWrapper()->CreateVertexBuffer(vbci);
	vaci.indexBuffer = model.index_buffer = engine.getGraphicsWrapper()->CreateIndexBuffer(ibci);
	model.vertex_array_object->BindResources(vaci);
	model.vertex_array_object->Unbind();

	// Create Final Submeshes
	for (unsigned int i = 0; i < inFormat.num_meshes; i++) {
		// Use the temporarily uint32_t material as an ID for the actual material.
		MeshCreateInfo &temp_mesh = temp_meshes[i];
		MeshStatic &current_mesh = model.meshes[i];
		current_mesh.model_reference = model.handle_;
		current_mesh.material_reference = material_references[temp_mesh.material_index];
		current_mesh.num_indices = temp_mesh.num_indices;
		current_mesh.base_vertex = temp_mesh.base_vertex;
		current_mesh.base_index = temp_mesh.base_index;
		auto mat = engine.getMaterialManager()->getMaterial(current_mesh.material_reference);
		mat->m_meshes.push_back(reinterpret_cast<MeshStatic *>(&current_mesh));
	}*/
}