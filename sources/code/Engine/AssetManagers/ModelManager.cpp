#include "ModelManager.hpp"
#include "MaterialManager.hpp"
#include "../Systems/RenderStaticMeshSystem.hpp"
#include "../Systems/TransformSystem.hpp"
#include "glm/common.hpp"

#include "Core/Utilities.hpp"
#include "Core/Engine.hpp"

#include "../GraphicsCommon/VertexBuffer.hpp"
#include <GraphicsCommon/GraphicsWrapper.hpp>

#include  "Core/Scene.hpp"
#include  "Core/Space.hpp"
#include  "Core/GameObject.hpp"
#include <fstream>

ModelStatic::ModelStatic() {}
ModelStatic::ModelStatic(ModelReference handle, std::string path, ComponentHandle ref) : handle_(handle), path_(path), index_buffer(nullptr), vertex_buffer(nullptr), vertex_array_object(nullptr), shadow_vertex_array_object(nullptr), shadow_vertex_buffer(nullptr) {
	references_.push_back(ref);
};

void MeshStatic::shadowDraw() {
	draw();
}

void MeshStatic::draw() {
	auto sp = engine.getSpace(0);
	RenderStaticMeshSubSystem *render_system = (RenderStaticMeshSubSystem *)sp->getSubsystem(COMPONENT_RENDER_STATIC_MESH);
	
	ModelStatic *model = &engine.getModelManager()->getModel(model_reference);
	for (auto &reference : model->references_) {
		RenderStaticMeshComponent &render_component = render_system->getComponent(reference);

		//if (renderComponent.should_draw) {
			auto game_object_id = render_component.game_object_handle_;
			auto game_object = sp->getObject(game_object_id);
			auto transform = game_object.getComponent<TransformComponent>();

			engine.getModelManager()->getModelUbo()->Bind();
			engine.getModelManager()->getModelUbo()->updateBuffer(&transform->getModelMatrix());

			engine.getGraphicsWrapper()->bindVertexArrayObject(model->vertex_array_object);
			engine.getGraphicsWrapper()->drawImmediateIndexed(Grindstone::GraphicsAPI::GeometryType::Triangles, true, base_vertex, base_index, num_indices);
		//}
	}
}

ModelManager::ModelManager(Grindstone::GraphicsAPI::UniformBufferBinding *ubb) {
	GRIND_PROFILE_FUNC();


	vertex_layout_ = Grindstone::GraphicsAPI::VertexBufferLayout({
		{ Grindstone::GraphicsAPI::VertexFormat::Float3, "vertexPosition", false, Grindstone::GraphicsAPI::AttributeUsage::Position },
		{ Grindstone::GraphicsAPI::VertexFormat::Float3, "vertexNormal", false, Grindstone::GraphicsAPI::AttributeUsage::Normal },
		{ Grindstone::GraphicsAPI::VertexFormat::Float3, "vertexTangent", false, Grindstone::GraphicsAPI::AttributeUsage::Tangent },
		{ Grindstone::GraphicsAPI::VertexFormat::Float2, "vertexTexCoord", false, Grindstone::GraphicsAPI::AttributeUsage::TexCoord0 }
	});

	// Fix this soon
	geometry_info_.vertex_layout = &vertex_layout_;
	geometry_info_.vertex_layout_count = 1;

	prepareGraphics();
}

void ModelManager::prepareGraphics() {
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

	ubbs_ = { engine.getUniformBufferBinding(), model_ubb_ };

	geometry_info_.ubb_count = (unsigned int)ubbs_.size();
	geometry_info_.ubbs = ubbs_.data();

	empty_material = engine.getMaterialManager()->loadMaterial(geometry_info_, "../engineassets/materials/missing.gmat");
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
	if (loadModel(models_.back())) {
		return ref;
	}
	else {
		models_.pop_back();
		return -1;
	}
}

void ModelManager::loadPreloaded() {
	for (auto model : unloaded_) {
		loadModel(models_[model]);
	}
}

ModelStatic &ModelManager::getModel(ModelReference reference) {
	return models_[reference];
}

Grindstone::GraphicsAPI::UniformBuffer * ModelManager::getModelUbo() {
	return model_ubo_;
}

void ModelManager::removeModelInstance(ModelReference model_ref, ComponentHandle c) {
	auto &m = models_[model_ref];
	auto &mr = m.references_;

	for (int i = 0; i < mr.size(); ++i) {
		if (mr[i] == c) mr.erase(mr.begin() + i);
	}

	if (mr.size() == 0) {
		for (auto &mesh : m.meshes) {
			engine.getMaterialManager()->removeMeshFromMaterial(mesh.material_reference, &mesh);
		}

		// Delete model data
		Grindstone::GraphicsAPI::GraphicsWrapper *gw = engine.getGraphicsWrapper();
		gw->deleteVertexArrayObject(m.vertex_array_object);
		gw->deleteVertexBuffer(m.vertex_buffer);
		gw->deleteIndexBuffer(m.index_buffer);

		models_.erase(models_.begin() + model_ref);
	}

	// UPDATE MODEL REFERENCE LIST
}

void ModelManager::destroyGraphics() {
	Grindstone::GraphicsAPI::GraphicsWrapper *gw = engine.getGraphicsWrapper();


	gw->deleteUniformBuffer(model_ubo_);
	gw->deleteUniformBufferBinding(model_ubb_);

	ubbs_.clear();
	for (ModelReference i = 0; i < (ModelReference)models_.size(); ++i) {
		destroyModel(i);
	}
}

void ModelManager::reloadGraphics() {
	prepareGraphics();

	for (ModelReference i = 0; i < (ModelReference)models_.size(); ++i) {
		loadModel(models_[i]);
	}
}

void ModelManager::destroyModel(ModelReference ref) {
	Grindstone::GraphicsAPI::GraphicsWrapper *gw = engine.getGraphicsWrapper();

	auto &m = models_[ref];
	// delete m.bounding;
	// m.command_buffer;
	if (m.loaded_) {
		if (m.vertex_buffer)
			gw->deleteVertexBuffer(m.vertex_buffer);
		if (m.vertex_array_object)
			gw->deleteVertexArrayObject(m.vertex_array_object);
		if (m.shadow_vertex_buffer)
			gw->deleteVertexBuffer(m.shadow_vertex_buffer);
		if (m.index_buffer)
			gw->deleteIndexBuffer(m.index_buffer);
		if (m.shadow_vertex_array_object)
			gw->deleteVertexArrayObject(m.shadow_vertex_array_object);
		m.meshes.clear();
	}
}

bool ModelManager::loadModel(ModelStatic &model) {
	Grindstone::GraphicsAPI::GraphicsWrapper *graphics_wrapper = engine.getGraphicsWrapper();
	MaterialManager *material_system = engine.getMaterialManager();

	std::string path = std::string("../assets/" + model.path_);
	std::ifstream input(path, std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		GRIND_ERROR("Failed to open file: {0}!", path);
		return false;
	}

	GRIND_LOG("Model reading from: {0}", path );

	size_t fileSize = (size_t)input.tellg();
	std::vector<char> buffer(fileSize);

	input.seekg(0);
	input.read(buffer.data(), fileSize);

	if (buffer[0] != 'G' || buffer[1] != 'M' || buffer[2] != 'F') {
		GRIND_ERROR("Failed to open file: {0}!", path);
		return false;
	}

	ModelFormatHeader inFormat;
	void *offset = buffer.data() + 3;
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
	uint64_t size64 = inFormat.num_vertices * sizeof(Vertex);
	memcpy(vertices.data(), offset, size64);
	offset = static_cast<char*>(offset) + size64;

	// Copy Vertex Weights
	if (inFormat.has_bones) {
		size64 = inFormat.num_vertices * sizeof(VertexWeights);
		memcpy(vertex_weights.data(), offset, size64);
		offset = static_cast<char*>(offset) + size64;
	}

	// Copy Indices
	size64 = inFormat.num_indices * sizeof(unsigned int);
	memcpy(indices.data(), offset, size64);
	offset = static_cast<char*>(offset) + size64;

	std::vector<MaterialReference> materialReferences;
	materialReferences.resize(inFormat.num_materials);

	std::vector<Material *> materials;
	materials.resize(inFormat.num_materials);
	char *words = (char *)offset;
	for (unsigned int i = 0; i < inFormat.num_materials; i++) {
		// Need to add a non-lazyload material
		materialReferences[i] = material_system->loadMaterial(geometry_info_, words);
		if (materialReferences[i].pipelineReference.pipeline_type == TYPE_MISSING) {
			materialReferences[i] = empty_material;
		}
		words = strchr(words, '\0') + 1;
	}

	for (unsigned int i = 0; i < inFormat.num_materials; i++) {
		materials[i] = material_system->getMaterial(materialReferences[i]);
	}

	input.close();

	Grindstone::GraphicsAPI::VertexBufferCreateInfo vbci;
	vbci.layout = &vertex_layout_;
	vbci.content = static_cast<const void *>(vertices.data());
	vbci.count = static_cast<uint32_t>(vertices.size());
	vbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());

	/*Grindstone::GraphicsAPI::VertexBufferCreateInfo svbci;
	svbci.attribute = geometry_info_.vads;
	svbci.attributeCount = (uint32_t)geometry_info_.vads_count;
	svbci.binding = geometry_info_.vbds;
	svbci.bindingCount = geometry_info_.vbds_count;
	svbci.content = static_cast<const void *>(vertices.data());
	svbci.count = static_cast<uint32_t>(vertices.size());
	svbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());*/

	Grindstone::GraphicsAPI::IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	model.vertex_buffer = graphics_wrapper->createVertexBuffer(vbci);
	model.index_buffer = graphics_wrapper->createIndexBuffer(ibci);

	if (graphics_wrapper->shouldUseImmediateMode()) {
		Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo vaci;
		vaci.vertex_buffers = &model.vertex_buffer;
		vaci.vertex_buffer_count = 1;
		vaci.index_buffer = model.index_buffer;
		model.vertex_array_object = graphics_wrapper->createVertexArrayObject(vaci);
		

		/*Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo vaci;
		vaci.vertexBuffer = model.shadowGrindstone::GraphicsAPI::VertexBuffer;
		vaci.indexBuffer = model.indexBuffer;
		model.shadowGrindstone::GraphicsAPI::VertexArrayObject = graphics_wrapper_->createVertexArrayObject(vaci);
		model.shadowGrindstone::GraphicsAPI::VertexBuffer = graphics_wrapper_->createVertexBuffer(svbci);
		model.indexBuffer = graphics_wrapper_->createIndexBuffer(ibci);

		vaci.vertexBuffer = model.shadowGrindstone::GraphicsAPI::VertexBuffer;
		vaci.indexBuffer = model.indexBuffer;
		model.shadowGrindstone::GraphicsAPI::VertexArrayObject->BindResources(vaci);
		model.shadowGrindstone::GraphicsAPI::VertexArrayObject->Unbind();*/

	}

	for (unsigned int i = 0; i < inFormat.num_meshes; i++) {
		// Use the temporarily uint32_t material as an ID for the actual material.
		MeshCreateInfo &temp_mesh = temp_meshes[i];
		MeshStatic &current_mesh = model.meshes[i];
		current_mesh.model_reference = model.handle_;
		current_mesh.material_reference = materialReferences[temp_mesh.material_index];
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
	std::
	<< "Header:" << a << "\n";
	
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
	Grindstone::GraphicsAPI::VertexBufferCreateInfo vbci;
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
	Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo vaci;
	vaci.vertexBuffer = model.vertex_buffer;
	vaci.indexBuffer = model.index_buffer;
	model.vertex_array_object = engine.getGraphicsWrapper()->createVertexArrayObject(vaci);
	vaci.vertexBuffer = model.vertex_buffer = engine.getGraphicsWrapper()->createVertexBuffer(vbci);
	vaci.indexBuffer = model.index_buffer = engine.getGraphicsWrapper()->createIndexBuffer(ibci);
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

	model.loaded_ = true;

	return true;
}