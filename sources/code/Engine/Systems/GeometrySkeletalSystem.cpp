#if 0
#include "../Core/Engine.h"

#include "SGeometry.h"
#include "../Core/TextureManager.h"
#include "../Core/GraphicsDLLPointer.h"

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

std::string CModel::getName() {
	return name;
}

void SModel::PreloadModel3D(const char * szPath, size_t renderID) {
	for (size_t i = 0; i < models.size(); i++) {
		if (models[i].getName() == szPath) {
			models[i].references.push_back((unsigned int)renderID);
			return;
		}
	}

	models.push_back(CModel());
	CModel *model = &models.back();
	model->references.push_back((unsigned int)renderID);
	model->name = szPath;
	unloadedModelIDs.push_back((unsigned int)(models.size() - 1));
}

void SModel::LoadPreloaded() {
	for (size_t i = 0; i < unloadedModelIDs.size(); i++) {
		CModel *model = &models[unloadedModelIDs[i]];
		LoadModel3DFile(model->getName().c_str(), model);
	}
}

void SModel::LoadModel3D(const char * szPath, size_t renderID) {
	for (size_t i = 0; i < models.size(); i++) {
		if (models[i].getName() == szPath) {
			renderID = models[i].references.size();
			models[i].references.push_back((unsigned int)renderID);
			return;
		}
	}

	renderID = 0;
	models.push_back(CModel());
	CModel *model = &models.back();
	model->references.push_back((unsigned int)renderID);
	model->name = szPath;
	LoadModel3DFile(szPath, model);
}

std::vector<CommandBuffer*> SModel::GetCommandBuffers() {
	std::vector<CommandBuffer*> buffers;
	buffers.reserve(models.size());
	for (CModel model : models) {
		buffers.push_back(model.commandBuffer);
	}

	return buffers;
}

/*void Mesh::Draw() {
Grindstone::GraphicsAPI::VertexArrayObject->Bind();
engine.graphics_wrapper_->DrawGrindstone::GraphicsAPI::VertexArrayObject(true, BaseVertex, BaseIndex, NumIndices);
}*/

bool SModel::LoadModel3DFile(const char *szPath, CModel *model) {
	std::string path = std::string(szPath);
	std::ifstream input(path, std::ios::ate | std::ios::binary);

	if (!input.is_open()) {
		std::cerr << "Failed to open file: " << path << "!\n";
		return false;
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
	uint32_t size = inFormat.numMeshes * sizeof(Mesh);
	memcpy(&model->meshes[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.numVertices * sizeof(Vertex);
	memcpy(&vertices[0], offset, size);
	offset = static_cast<char*>(offset) + size;
	size = inFormat.numIndices * sizeof(unsigned int);
	memcpy(&indices[0], offset, size);
	offset = static_cast<char*>(offset) + size;

	std::vector<MaterialReference> materialReferences;
	materialReferences.resize(inFormat.numMaterials);

	std::vector<Material *> materials;
	materials.resize(inFormat.numMaterials);
	char *words = (char *)offset;
	for (unsigned int i = 0; i < inFormat.numMaterials; i++) {
		materialReferences[i] = materialManager->CreateMaterial(words);
		words = strchr(words, '\0') + 1;
	}

	for (unsigned int i = 0; i < inFormat.numMaterials; i++) {
		materials[i] = materialManager->GetMaterial(materialReferences[i]);
	}

	input.close();

	Grindstone::GraphicsAPI::VertexBufferCreateInfo vbci;
	vbci.attribute = vads.data();
	vbci.attributeCount = (uint32_t)vads.size();
	vbci.binding = &vbd;
	vbci.bindingCount = 1;
	vbci.content = static_cast<const void *>(vertices.data());
	vbci.count = static_cast<uint32_t>(vertices.size());
	vbci.size = static_cast<uint32_t>(sizeof(Vertex) * vertices.size());

	IndexBufferCreateInfo ibci;
	ibci.content = static_cast<const void *>(indices.data());
	ibci.count = static_cast<uint32_t>(indices.size());
	ibci.size = static_cast<uint32_t>(sizeof(uint32_t) * indices.size());

	if (graphics_wrapper_->SupportsCommandBuffers()) {
		model->Grindstone::GraphicsAPI::VertexBuffer = graphics_wrapper_->CreateVertexBuffer(vbci);
		model->indexBuffer = graphics_wrapper_->CreateIndexBuffer(ibci);
	}
	else {
		Grindstone::GraphicsAPI::VertexArrayObjectCreateInfo vaci;
		vaci.vertexBuffer = model->Grindstone::GraphicsAPI::VertexBuffer;
		vaci.indexBuffer = model->indexBuffer;
		model->Grindstone::GraphicsAPI::VertexArrayObject = graphics_wrapper_->CreateVertexArrayObject(vaci);
		model->Grindstone::GraphicsAPI::VertexBuffer = graphics_wrapper_->CreateVertexBuffer(vbci);
		model->indexBuffer = graphics_wrapper_->CreateIndexBuffer(ibci);

		vaci.vertexBuffer = model->Grindstone::GraphicsAPI::VertexBuffer;
		vaci.indexBuffer = model->indexBuffer;
		model->Grindstone::GraphicsAPI::VertexArrayObject->BindResources(vaci);
		model->Grindstone::GraphicsAPI::VertexArrayObject->Unbind();
	}

	for (unsigned int i = 0; i < inFormat.numMeshes; i++) {
		// Use the temporarily uint32_t material as an ID for the actual material.
		Mesh *currentMesh = &model->meshes[i];
		uint16_t matID = currentMesh->material.material;
		currentMesh->material = materialReferences[matID];
		currentMesh->model = model;
		Material *mat = materials[matID];
		mat->m_meshes.push_back(currentMesh);
	}

	return true;
}

void SModel::Initialize(GraphicsWrapper * _graphics_wrapper_, Grindstone::GraphicsAPI::VertexBindingDescription _vbd, std::vector<Grindstone::GraphicsAPI::VertexAttributeDescription> _vads, MaterialManager *_materialSystem) {
	graphics_wrapper_ = _graphics_wrapper_;
	vbd = _vbd;
	vads = _vads;
	materialManager = _materialSystem;
}

void SModel::AddComponent(unsigned int entID, unsigned int &target) {
	renderComponents.push_back(CRender());
	renderComponents[renderComponents.size() - 1].entityID = entID;
	target = (unsigned int)(renderComponents.size() - 1);
}

void SModel::Shutdown() {
	if (graphics_wrapper_->SupportsCommandBuffers()) {
		for (size_t i = 0; i < models.size(); i++) {
			graphics_wrapper_->DeleteCommandBuffer(models[i].commandBuffer);
			graphics_wrapper_->DeleteVertexBuffer(models[i].vertexBuffer);
			graphics_wrapper_->DeleteIndexBuffer(models[i].indexBuffer);
		}
	}
	else {
		for (size_t i = 0; i < models.size(); i++) {
			graphics_wrapper_->DeleteVertexArrayObject(models[i].Grindstone::GraphicsAPI::VertexArrayObject);
			graphics_wrapper_->DeleteVertexBuffer(models[i].vertexBuffer);
			graphics_wrapper_->DeleteIndexBuffer(models[i].indexBuffer);
		}
	}
}

void Mesh::Draw() {
	engine.graphics_wrapper_->BindVertexArrayObject(model->Grindstone::GraphicsAPI::VertexArrayObject);
	engine.graphics_wrapper_->DrawImmediateIndexed(true, BaseVertex, BaseIndex, NumIndices);
}

void Mesh::DrawDeferred(CommandBuffer *cmd) {
	cmd->BindBufferObjects(model->Grindstone::GraphicsAPI::VertexBuffer, model->indexBuffer, model->useLargeBuffer);
	cmd->DrawIndexed(BaseVertex, BaseIndex, NumIndices, 1);
}
#endif