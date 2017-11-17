#include "SGeometryStatic.hpp"
#include "../Core/Engine.hpp"
#include <fstream>

struct ModelUBO {
	glm::mat4 pvmMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	int texLoc0;
	int texLoc1;
	int texLoc2;
	int texLoc3;
} ubo;

void MeshStatic::Draw() {
	for (auto &reference : model->references) {
		auto renderComponent = engine.geometry_system.GetComponent(reference);
		auto entityID = renderComponent.entity_id;
		auto transform = engine.transformSystem.components[entityID];
		glm::mat4 modelMatrix = transform.GetModelMatrix();

		engine.ubo2->UpdateUniformBuffer(&modelMatrix);
		engine.ubo2->Bind();

		engine.graphics_wrapper_->BindVertexArrayObject(model->vertexArrayObject);
		engine.graphics_wrapper_->DrawImmediateIndexed(true, BaseVertex, BaseIndex, NumIndices);
	}
}

void MeshStatic::DrawDeferred(CommandBuffer *cmd) {
	cmd->BindBufferObjects(model->vertexBuffer, model->indexBuffer, model->useLargeBuffer);
	cmd->DrawIndexed(BaseVertex, BaseIndex, NumIndices, 1);
}

std::string CModelStatic::getName() {
	return name;
}

void CModelStatic::setName(std::string path) {
	name = path;
}

void SGeometryStatic::LoadGeometry(uint32_t render_id, std::string path) {
	for (size_t i = 0; i < models.size(); i++) {
		if (models[i].getName() == path) {
			models[i].references.push_back((uint32_t)render_id);
			return;
		}
	}

	unloadedModelIDs.push_back((unsigned int)(models.size()));
	models.push_back({path, {render_id}});
}

void SGeometryStatic::LoadPreloaded() {
	for (unsigned int unloadedModelID : unloadedModelIDs) {
		CModelStatic *model = &models[unloadedModelID];
		LoadModel(model);
	}
}

std::vector<CommandBuffer*> SGeometryStatic::GetCommandBuffers() {
	std::vector<CommandBuffer*> buffers;
	buffers.reserve(models.size());
	for (CModelStatic model : models) {
		buffers.push_back(model.commandBuffer);
	}

	return buffers;
}

void SGeometryStatic::LoadModel(CModelStatic *model) {
	std::string path = std::string(model->getName());
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

	model->meshes.resize(inFormat.numMeshes);
	std::vector<Vertex> vertices;
	vertices.resize(inFormat.numVertices);
	std::vector<unsigned int> indices;
	indices.resize(inFormat.numIndices);

	offset = static_cast<char*>(offset) + sizeof(ModelFormatHeader);
	uint32_t size = inFormat.numMeshes * sizeof(MeshCreateInfo);
	std::vector<MeshCreateInfo> temp_meshes(inFormat.numMeshes);
	memcpy(temp_meshes.data(), offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.numVertices * sizeof(Vertex);
	memcpy(&vertices[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.numIndices * sizeof(unsigned int);
	memcpy(&indices[0], offset, size);
	offset = static_cast<char*>(offset) + size;

	std::cout << inFormat.numMaterials << "\n";
	std::vector<MaterialReference> materialReferences;
	materialReferences.resize(inFormat.numMaterials);

	std::vector<Material *> materials;
	materials.resize(inFormat.numMaterials);
	char *words = (char *)offset;
	for (unsigned int i = 0; i < inFormat.numMaterials; i++) {
		// Need to add a non-lazyload material
		std::cout << words << std::endl;
		materialReferences[i] = material_system_->CreateMaterial(words);
		words = strchr(words, '\0') + 1;
	}

	for (unsigned int i = 0; i < inFormat.numMaterials; i++) {
		materials[i] = material_system_->GetMaterial(materialReferences[i]);
	}

	input.close();

	VertexBufferCreateInfo vbci;
	vbci.attribute = vads_.data();
	vbci.attributeCount = (uint32_t)vads_.size();
	vbci.binding = &vbd_;
	vbci.bindingCount = 1;
	vbci.content = static_cast<const void *>(vertices.data());
	vbci.count = static_cast<uint32_t>(vertices.size());
	vbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());

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

	for (unsigned int i = 0; i < inFormat.numMeshes; i++) {
		// Use the temporarily uint32_t material as an ID for the actual material.
		MeshCreateInfo &temp_mesh = temp_meshes[i];
		MeshStatic &current_mesh = model->meshes[i];
		current_mesh.model = model;
		current_mesh.material = materials[temp_mesh.MaterialIndex];
		current_mesh.NumIndices = temp_mesh.NumIndices;
		current_mesh.BaseVertex = temp_mesh.BaseVertex;
		current_mesh.BaseIndex = temp_mesh.BaseIndex;
		current_mesh.material->m_meshes.push_back(reinterpret_cast<Mesh *>(&current_mesh));
	}
}

SGeometryStatic::SGeometryStatic(MaterialManager *material_system, GraphicsWrapper * graphics_wrapper, VertexBindingDescription vbd, std::vector<VertexAttributeDescription> vads) : graphics_wrapper_(graphics_wrapper), vbd_(vbd), vads_(vads), material_system_(material_system) {}

SGeometryStatic::~SGeometryStatic() {
	if (graphics_wrapper_) {
		if (graphics_wrapper_->SupportsCommandBuffers()) {
			for (size_t i = 0; i < models.size(); i++) {
				graphics_wrapper_->DeleteCommandBuffer(models[i].commandBuffer);
				graphics_wrapper_->DeleteVertexBuffer(models[i].vertexBuffer);
				graphics_wrapper_->DeleteIndexBuffer(models[i].indexBuffer);
			}
		}
		else {
			for (size_t i = 0; i < models.size(); i++) {
				graphics_wrapper_->DeleteVertexArrayObject(models[i].vertexArrayObject);
				graphics_wrapper_->DeleteVertexBuffer(models[i].vertexBuffer);
				graphics_wrapper_->DeleteIndexBuffer(models[i].indexBuffer);
			}
		}
	}
}