#include <filesystem>
#include "Mesh3dManager.hpp"
#include "EngineCore/Assets/Materials/MaterialManager.hpp"
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
	return EngineCore::GetInstance().GetGraphicsCore()->CreateVertexBuffer(vertexBufferCreateInfo);
}

Mesh3dManager::Mesh3dManager() {
	PrepareLayouts();
}

void Mesh3dManager::PrepareLayouts() {
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

Mesh3d& Mesh3dManager::LoadMesh3d(Uuid uuid) {
	Mesh3d* mesh = nullptr;
	if (TryGetMesh3d(uuid, mesh)) {
		mesh->useCount++;
		return *mesh;
	}

	return CreateMesh3dFromFile(uuid);
}

void Mesh3dManager::DecrementMeshCount(ECS::Entity entity, Uuid uuid) {
	auto meshInMap = meshes.find(uuid);
	if (meshInMap != meshes.end()) {
		auto mesh = &meshInMap->second;
		mesh->useCount -= 1;

		auto materialManager = EngineCore::GetInstance().materialManager;
		for (auto& submesh : mesh->submeshes) {
			for (auto& material : submesh.materials) {
				materialManager->RemoveRenderableFromMaterial(material, entity, &submesh);
			}
		}

		if (mesh->useCount == 0) {
			auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
			graphicsCore->DeleteVertexArrayObject(mesh->vertexArrayObject);
			meshes.erase(meshInMap);
		}
	}
}

bool Mesh3dManager::TryGetMesh3d(Uuid uuid, Mesh3d*& mesh) {
	auto meshInMap = meshes.find(uuid);
	if (meshInMap != meshes.end()) {
		mesh = &meshInMap->second;
		return true;
	}

	return false;
}

void Mesh3dManager::LoadMeshImportSubmeshes(Mesh3d& mesh, Formats::Model::Header::V1& header, char*& sourcePtr) {
	SourceSubmesh* sourceSubmeshes = (SourceSubmesh*)sourcePtr;
	mesh.submeshes.resize(header.meshCount);

	for (uint32_t i = 0; i < header.meshCount; ++i) {
		Mesh3d::Submesh& dst = mesh.submeshes[i];
		SourceSubmesh& src = sourceSubmeshes[i];
		dst.baseIndex = src.baseIndex;
		dst.baseVertex = src.baseVertex;
		dst.indexCount = src.indexCount;
		dst.materialIndex = src.materialIndex;
		dst.mesh = &mesh;
	}

	sourcePtr += header.meshCount * sizeof(SourceSubmesh);
}

void Mesh3dManager::LoadMeshImportVertices(
	Mesh3d& mesh,
	Formats::Model::Header::V1& header,
	char*& sourcePtr,
	std::vector<GraphicsAPI::VertexBuffer*>& vertexBuffers
) {
	auto fileName = mesh.uuid.ToString();
	auto vertexCount = header.vertexCount;
	if (header.hasVertexPositions) {
		auto positions = LoadVertexBufferVec(fileName, 3, vertexCount, sourcePtr, vertexLayouts.positions);
		vertexBuffers.push_back(positions);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexNormals) {
		auto normals = LoadVertexBufferVec(fileName, 3, vertexCount, sourcePtr, vertexLayouts.normals);
		vertexBuffers.push_back(normals);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexTangents) {
		auto tangents = LoadVertexBufferVec(fileName, 3, vertexCount, sourcePtr, vertexLayouts.tangents);
		vertexBuffers.push_back(tangents);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.vertexUvSetCount >= 1) {
		auto uv0 = LoadVertexBufferVec(fileName, 2, vertexCount, sourcePtr, vertexLayouts.uv0);
		vertexBuffers.push_back(uv0);
		sourcePtr += sizeof(float) * 2 * vertexCount;
	}
}

void Mesh3dManager::LoadMeshImportIndices(
	Mesh3d& mesh,
	Formats::Model::Header::V1& header,
	char*& sourcePtr,
	GraphicsAPI::IndexBuffer*& indexBuffer
) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
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

void Mesh3dManager::CreateMeshFromData(Mesh3d& mesh, std::vector<char>& fileContent) {
	auto graphicsCore = EngineCore::GetInstance().GetGraphicsCore();
	if (fileContent.size() < 3 && strncmp("GMF", fileContent.data(), 3) != 0) {
		throw std::runtime_error("Mesh3dManager::CreateMeshFromData GMF magic code wasn't matched.");
	}
	Formats::Model::Header::V1 header;
	if (fileContent.size() < (3 + sizeof(header))) {
		throw std::runtime_error("Mesh3dManager::CreateMeshFromData file not big enough to fit header.");
	}
	char* headerPtr = fileContent.data() + 3;
	header = *(Formats::Model::Header::V1*)headerPtr;

	char* srcPtr = headerPtr + sizeof(header);

	std::vector<GraphicsAPI::VertexBuffer*> vertexBuffers;
	GraphicsAPI::IndexBuffer* indexBuffer = nullptr;

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
}

Mesh3d& Mesh3dManager::CreateMesh3dFromFile(Uuid uuid) {
	std::string completePath = std::string("../compiledAssets/") + uuid.ToString();
	if (!std::filesystem::exists(completePath)) {
		throw std::runtime_error("Mesh3dManager::CreateMesh3dFromFile failed to load model.");
	}

	auto fileContent = Utils::LoadFile(completePath.c_str());
	Mesh3d& mesh = meshes[uuid];
	mesh.uuid = uuid;
	CreateMeshFromData(mesh, fileContent);

	return mesh;
}
