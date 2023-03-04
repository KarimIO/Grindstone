#include <filesystem>
#include "Mesh3dImporter.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/Assets/Materials/MaterialImporter.hpp"
#include "EngineCore/EngineCore.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/Graphics/Core.hpp"
using namespace Grindstone;

struct SourceSubmesh {
	uint32_t indexCount = 0;
	uint32_t baseVertex = 0;
	uint32_t baseIndex = 0;
	uint32_t materialIndex = UINT32_MAX;
};

GraphicsAPI::VertexBuffer* LoadVertexBufferVec(
	GraphicsAPI::Core* graphicsCore,
	std::string& fileName,
	size_t vertexSize,
	uint64_t vertexCount,
	void* sourcePtr,
	GraphicsAPI::VertexBufferLayout& vertexLayout
) {
	size_t size = sizeof(float) * vertexSize * vertexCount;
	std::vector<float> vertices;
	vertices.resize(vertexCount * vertexSize);
	std::memcpy(vertices.data(), sourcePtr, size);

	std::string debugName = fileName + " " + vertexLayout.attributes[0].name;
	GraphicsAPI::VertexBuffer::CreateInfo vertexBufferCreateInfo;
	vertexBufferCreateInfo.debugName = debugName.c_str();
	vertexBufferCreateInfo.content = vertices.data();
	vertexBufferCreateInfo.size = size;
	vertexBufferCreateInfo.layout = &vertexLayout;
	vertexBufferCreateInfo.count = vertexCount;
	return graphicsCore->CreateVertexBuffer(vertexBufferCreateInfo);
}

Mesh3dImporter::Mesh3dImporter() {
	PrepareLayouts();
}

void Mesh3dImporter::PrepareLayouts() {
	vertexLayouts.positions = {
		{
			(uint32_t)Mesh3dLayoutIndex::Position,
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexPosition",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Position
		}
	};

	vertexLayouts.normals = {
		{
			(uint32_t)Mesh3dLayoutIndex::Normal,
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexNormal",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Normal
		}
	};

	vertexLayouts.tangents = {
		{
			(uint32_t)Mesh3dLayoutIndex::Tangent,
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexTangent",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Tangent
		}
	};

	vertexLayouts.uv0 = {
		{
			(uint32_t)Mesh3dLayoutIndex::Uv0,
			Grindstone::GraphicsAPI::VertexFormat::Float2,
			"vertexTexCoord0",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::TexCoord0
		}
	};

	vertexLayouts.uv1 = {
		{
			(uint32_t)Mesh3dLayoutIndex::Uv1,
			Grindstone::GraphicsAPI::VertexFormat::Float2,
			"vertexTexCoord1",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::TexCoord1
		}
	};
}

void Mesh3dImporter::DecrementMeshCount(ECS::Entity entity, Uuid uuid) {
	auto meshInMap = meshes.find(uuid);
	if (meshInMap == meshes.end()) {
		return;
	}

	auto mesh = &meshInMap->second;
	mesh->referenceCount -= 1;

	/*
	auto materialManager = engineCore.materialImporter;
	for (auto& submesh : mesh->submeshes) {
		for (auto& material : submesh.materials) {
			materialManager->RemoveRenderableFromMaterial(material, entity, &submesh);
		}
	}

	if (mesh->referenceCount == 0) {
		auto graphicsCore = engineCore.GetGraphicsCore();
		graphicsCore->DeleteVertexArrayObject(mesh->vertexArrayObject);
		meshes.erase(meshInMap);
	}
	*/
}

bool Mesh3dImporter::TryGetIfLoaded(Uuid uuid, void*& mesh) {
	auto meshInMap = meshes.find(uuid);
	if (meshInMap != meshes.end()) {
		mesh = &meshInMap->second;
		return true;
	}

	return false;
}

void Mesh3dImporter::LoadMeshImportSubmeshes(Mesh3dAsset& mesh, Formats::Model::V1::Header& header, char*& sourcePtr) {
	SourceSubmesh* sourceSubmeshes = (SourceSubmesh*)sourcePtr;
	mesh.submeshes.resize(header.meshCount);

	for (uint32_t i = 0; i < header.meshCount; ++i) {
		Mesh3dAsset::Submesh& dst = mesh.submeshes[i];
		SourceSubmesh& src = sourceSubmeshes[i];
		dst.baseIndex = src.baseIndex;
		dst.baseVertex = src.baseVertex;
		dst.indexCount = src.indexCount;
		dst.materialIndex = src.materialIndex;
	}

	sourcePtr += header.meshCount * sizeof(SourceSubmesh);
}

void Mesh3dImporter::LoadMeshImportVertices(
	Mesh3dAsset& mesh,
	Formats::Model::V1::Header& header,
	char*& sourcePtr,
	std::vector<GraphicsAPI::VertexBuffer*>& vertexBuffers
) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	auto fileName = mesh.uuid.ToString();
	auto vertexCount = header.vertexCount;
	if (header.hasVertexPositions) {
		auto positions = LoadVertexBufferVec(graphicsCore, fileName, 3, vertexCount, sourcePtr, vertexLayouts.positions);
		vertexBuffers.push_back(positions);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexNormals) {
		auto normals = LoadVertexBufferVec(graphicsCore, fileName, 3, vertexCount, sourcePtr, vertexLayouts.normals);
		vertexBuffers.push_back(normals);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexTangents) {
		auto tangents = LoadVertexBufferVec(graphicsCore, fileName, 3, vertexCount, sourcePtr, vertexLayouts.tangents);
		vertexBuffers.push_back(tangents);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.vertexUvSetCount >= 1) {
		auto uv0 = LoadVertexBufferVec(graphicsCore, fileName, 2, vertexCount, sourcePtr, vertexLayouts.uv0);
		vertexBuffers.push_back(uv0);
		sourcePtr += sizeof(float) * 2 * vertexCount;
	}
}

void Mesh3dImporter::LoadMeshImportIndices(
	Mesh3dAsset& mesh,
	Formats::Model::V1::Header& header,
	char*& sourcePtr,
	GraphicsAPI::IndexBuffer*& indexBuffer
) {
	auto graphicsCore = engineCore->GetGraphicsCore();
	std::vector<uint16_t> indices;
	indices.resize(header.indexCount);
	uint32_t indexSize = header.indexCount * sizeof(uint16_t);
	memcpy(indices.data(), sourcePtr, indexSize);
	sourcePtr += indexSize;

	std::string fileName = mesh.uuid.ToString();
	std::string debugName = fileName + " Index Buffer";
	GraphicsAPI::IndexBuffer::CreateInfo indexBufferCreateInfo{};
	indexBufferCreateInfo.debugName = debugName.c_str();
	indexBufferCreateInfo.content = indices.data();
	indexBufferCreateInfo.count = indices.size();
	indexBufferCreateInfo.size = indices.size() * sizeof(indices[0]);
	indexBuffer = graphicsCore->CreateIndexBuffer(indexBufferCreateInfo);
}

void* Mesh3dImporter::ProcessLoadedFile(Uuid uuid) {
	char* fileContent;
	size_t fileSize;
	if (!engineCore->assetManager->LoadFile(uuid, fileContent, fileSize)) {
		return nullptr;
	}

	auto graphicsCore = engineCore->GetGraphicsCore();
	if (fileSize < 3 && strncmp("GMF", fileContent, 3) != 0) {
		throw std::runtime_error("Mesh3dImporter::CreateMeshFromData GMF magic code wasn't matched.");
	}
	Formats::Model::V1::Header header;
	if (fileSize < (3 + sizeof(header))) {
		throw std::runtime_error("Mesh3dImporter::CreateMeshFromData file not big enough to fit header.");
	}
	char* headerPtr = fileContent + 3;
	header = *(Formats::Model::V1::Header*)headerPtr;

	char* srcPtr = headerPtr + sizeof(header);

	std::vector<GraphicsAPI::VertexBuffer*> vertexBuffers;
	GraphicsAPI::IndexBuffer* indexBuffer = nullptr;

	auto& meshIterator = meshes.emplace(uuid, Mesh3dAsset(uuid, uuid.ToString()));
	Mesh3dAsset& mesh = meshIterator.first->second;

	Mesh3dAsset* meshAsset = nullptr;
	LoadMeshImportSubmeshes(mesh, header, srcPtr);
	LoadMeshImportVertices(mesh, header, srcPtr, vertexBuffers);
	LoadMeshImportIndices(mesh, header, srcPtr, indexBuffer);

	std::string fileName = mesh.uuid.ToString();
	std::string debugName = fileName + " Vertex Array Object";
	GraphicsAPI::VertexArrayObject::CreateInfo vaoCi{};
	vaoCi.debugName = debugName.c_str();
	vaoCi.indexBuffer = indexBuffer;
	vaoCi.vertexBuffers = vertexBuffers.data();
	vaoCi.vertexBufferCount = vertexBuffers.size();
	mesh.vertexArrayObject = graphicsCore->CreateVertexArrayObject(vaoCi);

	return &mesh;
}

EngineCore* Mesh3dImporter::engineCore;
