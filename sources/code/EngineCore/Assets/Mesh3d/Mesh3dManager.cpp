#include <filesystem>
#include "Mesh3dManager.hpp"
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
	size_t vertexSize,
	uint64_t vertexCount,
	void* sourcePtr,
	GraphicsAPI::VertexBufferLayout& vertexLayout
) {
	size_t size = sizeof(float) * vertexSize * vertexCount;
	std::vector<float> vertices;
	vertices.resize(vertexCount * vertexSize);
	std::memcpy(vertices.data(), sourcePtr, size);

	GraphicsAPI::VertexBuffer::CreateInfo vertexBufferCreateInfo;
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
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexPositions",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Position
		}
	};
	
	vertexLayouts.normals = {
		{
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexNormals",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Normal
		}
	};

	vertexLayouts.tangents = {
		{
			Grindstone::GraphicsAPI::VertexFormat::Float3,
			"vertexTangents",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::Tangent
		}
	};

	vertexLayouts.uv0 = {
		{
			Grindstone::GraphicsAPI::VertexFormat::Float2,
			"vertexUv0",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::TexCoord0
		}
	};

	vertexLayouts.uv1 = {
		{
			Grindstone::GraphicsAPI::VertexFormat::Float2,
			"vertexUv1",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::TexCoord1
		}
	};
}

Mesh3d& Mesh3dManager::LoadMesh3d(const char* path) {
	Mesh3d* mesh = nullptr;
	if (TryGetMesh3d(path, mesh)) {
		return *mesh;
	}

	return CreateMesh3dFromFile(path);
}

bool Mesh3dManager::TryGetMesh3d(const char* path, Mesh3d*& mesh) {
	auto& meshInMap = meshes.find(path);
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
	auto vertexCount = header.vertexCount;
	if (header.hasVertexPositions) {
		auto positions = LoadVertexBufferVec(3, vertexCount, sourcePtr, vertexLayouts.positions);
		vertexBuffers.push_back(positions);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexNormals) {
		auto normals = LoadVertexBufferVec(3, vertexCount, sourcePtr, vertexLayouts.normals);
		vertexBuffers.push_back(normals);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexTangents) {
		auto tangents = LoadVertexBufferVec(3, vertexCount, sourcePtr, vertexLayouts.tangents);
		vertexBuffers.push_back(tangents);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexTexCoord0) {
		auto uv0 = LoadVertexBufferVec(2, vertexCount, sourcePtr, vertexLayouts.uv0);
		vertexBuffers.push_back(uv0);
		sourcePtr += sizeof(float) * 2 * vertexCount;
	}

	if (header.hasVertexTexCoord1) {
		auto uv1 = LoadVertexBufferVec(2, vertexCount, sourcePtr, vertexLayouts.uv1);
		vertexBuffers.push_back(uv1);
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

	GraphicsAPI::IndexBuffer::CreateInfo indexBufferCreateInfo{};
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

	GraphicsAPI::VertexArrayObject::CreateInfo vaoCi{};
	vaoCi.indexBuffer = indexBuffer;
	vaoCi.vertexBuffers = vertexBuffers.data();
	vaoCi.vertexBufferCount = vertexBuffers.size();
	mesh.vertexArrayObject = graphicsCore->CreateVertexArrayObject(vaoCi);
}

Mesh3d& Mesh3dManager::CreateMesh3dFromFile(const char* path) {
	std::string completePath = std::string("../assets/") + path;
	if (!std::filesystem::exists(completePath)) {
		throw std::runtime_error("Mesh3dManager::CreateMesh3dFromFile failed to load model.");
	}

	auto fileContent = Utils::LoadFile(completePath.c_str());
	Mesh3d& mesh = meshes[path];
	CreateMeshFromData(mesh, fileContent);

	return mesh;
}
