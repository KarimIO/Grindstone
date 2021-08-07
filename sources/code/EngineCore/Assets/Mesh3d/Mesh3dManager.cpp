#include <filesystem>
#include "Mesh3dManager.hpp"
#include "EngineCore/Utils/Utilities.hpp"
using namespace Grindstone;

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
	mesh.submeshes.resize(header.meshCount);
	uint32_t submeshesSize = header.meshCount * sizeof(Mesh3d::Submesh);
	memcpy(mesh.submeshes.data(), sourcePtr, submeshesSize);
	sourcePtr += submeshesSize;
}

void Mesh3dManager::LoadMeshImportVertices(Mesh3d& mesh, Formats::Model::Header::V1& header, char*& sourcePtr) {
	/*if (header.hasVertexPositions) {
		LoadVertexBufferVec3(vertexCount, sourcePtr, vertexLayouts.positions);
	}

	if (header.hasVertexNormals) {
		LoadVertexBufferVec3(vertexCount, sourcePtr, vertexLayouts.normals);
	}

	if (header.hasVertexTangents) {
		LoadVertexBufferVec3(vertexCount, sourcePtr, vertexLayouts.tangents);
	}

	if (header.hasVertexTexCoord0) {
		LoadVertexBufferVec2(vertexCount, sourcePtr, vertexLayouts.uv0);
	}

	if (header.hasVertexTexCoord1) {
		LoadVertexBufferVec2(vertexCount, sourcePtr, vertexLayouts.uv1);
	}


	GraphicsAPI::VertexBuffer::CreateInfo vboCi{};
	vboCi.content = cubeVertexPositions.data();
	vboCi.count = cubeVertexPositions.size();
	vboCi.size = vboCi.count * sizeof(glm::vec3);
	vboCi.layout = &vertexPositionLayout;
	vbo = core->CreateVertexBuffer(vboCi);

	GraphicsAPI::VertexBufferLayout vertexTexCoordLayout({
		{
			Grindstone::GraphicsAPI::VertexFormat::Float2,
			"vertexTexCoord",
			false,
			Grindstone::GraphicsAPI::AttributeUsage::TexCoord0
		}
	});

	/*VertexBuffer::CreateInfo vboTexCi{};
	vboTexCi.content = cubeVertexTexCoords.data();
	vboTexCi.count = cubeVertexTexCoords.size();
	vboTexCi.size = vboTexCi.count * sizeof(glm::vec3);
	vboTexCi.layout = &vertexTexCoordLayout;
	vboTex = core->CreateVertexBuffer(vboTexCi);

	std::vector<float> indices;
	indices.resize(header.indexCount);
	uint32_t indexSize = header.indexCount * sizeof(uint32_t);
	memcpy(indices.data(), sourcePtr, indexSize);
	sourcePtr += indexSize;*/
}

void Mesh3dManager::LoadMeshImportIndices(Mesh3d& mesh, Formats::Model::Header::V1& header, char*& sourcePtr) {
	std::vector<uint32_t> indices;
	indices.resize(header.indexCount);
	uint32_t indexSize = header.indexCount * sizeof(uint32_t);
	memcpy(indices.data(), sourcePtr, indexSize);
	sourcePtr += indexSize;
}

void Mesh3dManager::CreateMeshFromData(Mesh3d& mesh, std::vector<char>& fileContent) {
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
	LoadMeshImportSubmeshes(mesh, header, srcPtr);
	LoadMeshImportVertices(mesh, header, srcPtr);
	LoadMeshImportIndices(mesh, header, srcPtr);

}

Mesh3d& Mesh3dManager::CreateMesh3dFromFile(const char* path) {
	if (!std::filesystem::exists(path)) {
		throw std::runtime_error("Mesh3dManager::CreateMesh3dFromFile failed to load model.");
	}

	auto fileContent = Utils::LoadFile(path);
	Mesh3d& mesh = meshes[path];
	CreateMeshFromData(mesh, fileContent);

	return mesh;
}
