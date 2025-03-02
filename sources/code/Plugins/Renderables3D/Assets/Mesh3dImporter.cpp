#include <filesystem>

#include <Common/Logging.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Assets/Materials/MaterialImporter.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>

#include "Mesh3dImporter.hpp"
using namespace Grindstone;

struct SourceSubmesh {
	uint32_t indexCount = 0;
	uint32_t baseVertex = 0;
	uint32_t baseIndex = 0;
	uint32_t materialIndex = UINT32_MAX;
};

static GraphicsAPI::VertexBuffer* LoadVertexBufferVec(
	GraphicsAPI::Core* graphicsCore,
	std::string& fileName,
	size_t vertexSize,
	uint64_t vertexCount,
	void* sourcePtr,
	GraphicsAPI::VertexBufferLayout& vertexLayout
) {
	uint64_t size = sizeof(float) * vertexSize * vertexCount;
	std::vector<float> vertices;
	vertices.resize(vertexCount * vertexSize);
	std::memcpy(vertices.data(), sourcePtr, size);

	std::string debugName = fileName + " " + vertexLayout.attributes[0].name;
	GraphicsAPI::VertexBuffer::CreateInfo vertexBufferCreateInfo;
	vertexBufferCreateInfo.debugName = debugName.c_str();
	vertexBufferCreateInfo.content = vertices.data();
	vertexBufferCreateInfo.size = static_cast<uint32_t>(size);
	vertexBufferCreateInfo.layout = &vertexLayout;
	vertexBufferCreateInfo.count = static_cast<uint32_t>(vertexCount);
	return graphicsCore->CreateVertexBuffer(vertexBufferCreateInfo);
}

Mesh3dImporter::Mesh3dImporter(EngineCore* engineCore) : engineCore(engineCore) {
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
	auto meshInMap = assets.find(uuid);
	if (meshInMap == assets.end()) {
		return;
	}

	auto mesh = &meshInMap->second;
	mesh->referenceCount -= 1;
}

void Mesh3dImporter::QueueReloadAsset(Uuid uuid) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	auto meshInMap = assets.find(uuid);
	if (meshInMap == assets.end()) {
		return;
	}

	auto& meshAsset = meshInMap->second;
	if (meshAsset.vertexArrayObject != nullptr) {
		graphicsCore->DeleteVertexArrayObject(meshAsset.vertexArrayObject);
	}

	meshAsset.submeshes.clear();

	ImportModelFile(meshAsset);
}

void Mesh3dImporter::LoadMeshImportSubmeshes(Mesh3dAsset& mesh, Formats::Model::V1::Header& header, char*& sourcePtr) {
	SourceSubmesh* sourceSubmeshes = reinterpret_cast<SourceSubmesh*>(sourcePtr);
	mesh.submeshes.resize(header.meshCount);

	for (uint32_t i = 0; i < header.meshCount; ++i) {
		Mesh3dAsset::Submesh& dst = mesh.submeshes[i];
		SourceSubmesh& src = sourceSubmeshes[i];
		dst.baseIndex = src.baseIndex;
		dst.baseVertex = src.baseVertex;
		dst.indexCount = src.indexCount;
		dst.materialIndex = src.materialIndex;
		dst.vertexArrayObject = nullptr;
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
	std::string& assetName = mesh.name;
	auto vertexCount = header.vertexCount;
	if (header.hasVertexPositions) {
		auto positions = LoadVertexBufferVec(graphicsCore, assetName, 3, vertexCount, sourcePtr, vertexLayouts.positions);
		vertexBuffers.push_back(positions);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexNormals) {
		auto normals = LoadVertexBufferVec(graphicsCore, assetName, 3, vertexCount, sourcePtr, vertexLayouts.normals);
		vertexBuffers.push_back(normals);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexTangents) {
		auto tangents = LoadVertexBufferVec(graphicsCore, assetName, 3, vertexCount, sourcePtr, vertexLayouts.tangents);
		vertexBuffers.push_back(tangents);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.vertexUvSetCount >= 1) {
		auto uv0 = LoadVertexBufferVec(graphicsCore, assetName, 2, vertexCount, sourcePtr, vertexLayouts.uv0);
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
	uint64_t indexSize = header.indexCount * sizeof(uint16_t);
	memcpy(indices.data(), sourcePtr, indexSize);
	sourcePtr += indexSize;

	std::string debugName = mesh.name + " Index Buffer";
	GraphicsAPI::IndexBuffer::CreateInfo indexBufferCreateInfo{};
	indexBufferCreateInfo.debugName = debugName.c_str();
	indexBufferCreateInfo.content = indices.data();
	indexBufferCreateInfo.count = static_cast<uint32_t>(indices.size());
	indexBufferCreateInfo.size = static_cast<uint32_t>(indices.size() * sizeof(indices[0]));
	indexBufferCreateInfo.is32Bit = false;
	indexBuffer = graphicsCore->CreateIndexBuffer(indexBufferCreateInfo);
}

void* Mesh3dImporter::LoadAsset(Uuid uuid) {
	auto& meshIterator = assets.emplace(uuid, Mesh3dAsset(uuid, uuid.ToString()));
	Mesh3dAsset& mesh = meshIterator.first->second;

	ImportModelFile(mesh);

	return &mesh;
}

bool Mesh3dImporter::ImportModelFile(Mesh3dAsset& mesh) {
	Grindstone::Assets::AssetLoadBinaryResult result = engineCore->assetManager->LoadBinaryByUuid(AssetType::Texture, mesh.uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Mesh3dImporter::LoadAsset Unable to load file with id: {}", mesh.uuid.ToString());
		return false;
	}

	char* fileContent = reinterpret_cast<char*>(result.buffer.Get());
	uint64_t fileSize = result.buffer.GetCapacity();
	mesh.name = result.displayName;

	auto graphicsCore = engineCore->GetGraphicsCore();
	if (fileSize < 3 && strncmp("GMF", fileContent, 3) != 0) {
		std::string errorMsg = "Mesh3dImporter::LoadAsset GMF magic code wasn't matched.";
		GPRINT_ERROR(LogSource::EngineCore, errorMsg.c_str());
		return false;
	}

	Formats::Model::V1::Header header;
	if (fileSize < (3 + sizeof(header))) {
		std::string errorMsg = "Mesh3dImporter::LoadAsset file not big enough to fit header.";
		GPRINT_ERROR(LogSource::EngineCore, errorMsg.c_str());
		return false;
	}
	char* headerPtr = fileContent + 3;
	header = *(Formats::Model::V1::Header*)headerPtr;

	char* srcPtr = headerPtr + sizeof(header);

	uint64_t totalFileExpectedSize = GetTotalFileSize(header);
	if (totalFileExpectedSize > fileSize || header.totalFileSize > fileSize) {
		std::string errorMsg = "Mesh3dImporter::LoadAsset file not big enough to fit all contents.";
		GPRINT_ERROR(LogSource::EngineCore, errorMsg.c_str());
		return false;
	}

	std::vector<GraphicsAPI::VertexBuffer*> vertexBuffers;
	GraphicsAPI::IndexBuffer* indexBuffer = nullptr;

	mesh.boundingData = *(Formats::Model::V1::BoundingData*)srcPtr;
	srcPtr = srcPtr + sizeof(Formats::Model::V1::BoundingData);

	LoadMeshImportSubmeshes(mesh, header, srcPtr);
	LoadMeshImportVertices(mesh, header, srcPtr, vertexBuffers);
	LoadMeshImportIndices(mesh, header, srcPtr, indexBuffer);

	std::string debugName = result.displayName + " Vertex Array Object";
	GraphicsAPI::VertexArrayObject::CreateInfo vaoCi{};
	vaoCi.debugName = debugName.c_str();
	vaoCi.indexBuffer = indexBuffer;
	vaoCi.vertexBuffers = vertexBuffers.data();
	vaoCi.vertexBufferCount = static_cast<uint32_t>(vertexBuffers.size());
	mesh.vertexArrayObject = graphicsCore->CreateVertexArrayObject(vaoCi);

	for (size_t i = 0; i < mesh.submeshes.size(); ++i)
	{
		mesh.submeshes[i].vertexArrayObject = mesh.vertexArrayObject;
	}

	return true;
}

uint64_t Mesh3dImporter::GetTotalFileSize(Formats::Model::V1::Header& header) {
	uint64_t totalFileExpectedSize = 3 + sizeof(header);

	if (header.hasVertexPositions) {
		totalFileExpectedSize += header.vertexCount * 3 * sizeof(float);
	}

	if (header.hasVertexNormals) {
		totalFileExpectedSize += header.vertexCount * 3 * sizeof(float);
	}

	if (header.hasVertexTangents) {
		totalFileExpectedSize += header.vertexCount * 3 * sizeof(float);
	}

	totalFileExpectedSize += header.vertexCount * header.vertexUvSetCount * 2 * sizeof(float);

	if (header.numWeightPerBone) {
		totalFileExpectedSize += header.vertexCount * 4 * sizeof(float);
	}

	totalFileExpectedSize += header.indexCount * sizeof(uint16_t);

	return totalFileExpectedSize;
}

Mesh3dImporter::~Mesh3dImporter() {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();

	for (auto& asset : assets) {
		graphicsCore->DeleteVertexArrayObject(asset.second.vertexArrayObject);
	}
	assets.clear();
}

Grindstone::Mesh3dImporter::VertexLayouts Mesh3dImporter::vertexLayouts;
