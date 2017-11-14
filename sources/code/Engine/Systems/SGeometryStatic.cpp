#include "SGeometryStatic.h"
#include "Core/Engine.h"
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
		auto renderComponent = engine.geometry_system.render_components[reference];
		auto entityID = renderComponent.entityID;
		auto transform = engine.transformSystem.components[entityID];
		glm::mat4 modelMatrix = transform.GetModelMatrix();

		engine.modelUBO.model = modelMatrix;
		engine.ubo2->UpdateUniformBuffer(&engine.modelUBO);
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

CGeometry *SGeometryStatic::PreloadComponent(std::string path) {
	for (size_t i = 0; i < models.size(); i++) {
		if (models[i].getName() == path) {
			models[i].references.push_back((unsigned int)renderID);
			return;
		}
	}

	models.push_back(CModelStatic());
	CModelStatic &model = models.back();
	model.references.push_back((unsigned int)renderID);
	model.name = path;
	unloadedModelIDs.push_back((unsigned int)(models.size() - 1));
}

CGeometry *SGeometryStatic::LoadComponent(std::string path) {
	for (size_t i = 0; i < models.size(); i++) {
		if (models[i].getName() == path) {
			renderID = models[i].references.size();
			models[i].references.push_back((unsigned int)renderID);
			return;
		}
	}

	renderID = 0;
	models.push_back(CModelStatic());
	CModelStatic &model = models.back();
	model.references.push_back((unsigned int)renderID);
	model.name = path;
	LoadModel(&model);
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
	uint32_t size = inFormat.numMeshes * sizeof(MeshStatic);
	memcpy(&model->meshes[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.numVertices * sizeof(Vertex);
	memcpy(&vertices[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.numIndices * sizeof(unsigned int);
	memcpy(&indices[0], offset, size);
	offset = static_cast<char*>(offset) + size;

	std::cout << inFormat.numMaterials << "\n";
	std::cin.get();
	std::vector<MaterialReference> materialReferences;
	materialReferences.resize(inFormat.numMaterials);

	std::vector<Material *> materials;
	materials.resize(inFormat.numMaterials);
	char *words = (char *)offset;
	for (unsigned int i = 0; i < inFormat.numMaterials; i++) {
		// Need to add a non-lazyload material
		materialReferences[i] = material_system_->PreLoadMaterial(words);
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
		MeshStatic *current_mesh = &model->meshes[i];
		uint16_t matID = current_mesh->material.material;
		current_mesh->material = materialReferences[matID];
		current_mesh->model = model;
		Material *mat = materials[matID];
		mat->m_meshes.push_back(reinterpret_cast<Mesh *>(current_mesh));
	}
}

SGeometryStatic::SGeometryStatic(GraphicsWrapper * graphics_wrapper) : graphics_wrapper_(graphics_wrapper) {
	vbd_.binding = 0;
	vbd_.elementRate = false;
	vbd_.stride = sizeof(Vertex);

	vads_.resize(4);
	vads_[0].binding = 0;
	vads_[0].location = 0;
	vads_[0].format = VERTEX_R32_G32_B32;
	vads_[0].size = 3;
	vads_[0].name = "vertexPosition";
	vads_[0].offset = offsetof(Vertex, positions);
	vads_[0].usage = ATTRIB_POSITION;

	vads_[1].binding = 0;
	vads_[1].location = 1;
	vads_[1].format = VERTEX_R32_G32_B32;
	vads_[1].size = 3;
	vads_[1].name = "vertexNormal";
	vads_[1].offset = offsetof(Vertex, normal);
	vads_[1].usage = ATTRIB_NORMAL;

	vads_[2].binding = 0;
	vads_[2].location = 2;
	vads_[2].format = VERTEX_R32_G32_B32;
	vads_[2].size = 3;
	vads_[2].name = "vertexTangent";
	vads_[2].offset = offsetof(Vertex, tangent);
	vads_[2].usage = ATTRIB_TANGENT;

	vads_[3].binding = 0;
	vads_[3].location = 3;
	vads_[3].format = VERTEX_R32_G32;
	vads_[3].size = 2;
	vads_[3].name = "vertexTexCoord";
	vads_[3].offset = offsetof(Vertex, texCoord);
	vads_[3].usage = ATTRIB_TEXCOORD0;
}

void SGeometryStatic::AddComponent(unsigned int entID, unsigned int &target) {
	renderComponents.push_back(CRender());
	renderComponents[renderComponents.size() - 1].entityID = entID;
	target = (unsigned int)(renderComponents.size() - 1);
}

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