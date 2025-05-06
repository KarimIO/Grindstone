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

static GraphicsAPI::Buffer* LoadVertexBufferVec(
	GraphicsAPI::Core* graphicsCore,
	std::string& fileName,
	size_t vertexSize,
	uint64_t vertexCount,
	void* sourcePtr,
	const char* bufferName
) {
	uint64_t size = sizeof(float) * vertexSize * vertexCount;
	std::vector<float> vertices;
	vertices.resize(vertexCount * vertexSize);
	std::memcpy(vertices.data(), sourcePtr, size);

	std::string debugName = fileName + " " + bufferName;
	GraphicsAPI::Buffer::CreateInfo vertexBufferCreateInfo{};
	vertexBufferCreateInfo.debugName = debugName.c_str();
	vertexBufferCreateInfo.content = vertices.data();
	vertexBufferCreateInfo.bufferUsage = Grindstone::GraphicsAPI::BufferUsage::Vertex;
	vertexBufferCreateInfo.bufferSize = static_cast<uint32_t>(size);
	return graphicsCore->CreateBuffer(vertexBufferCreateInfo);
}

Mesh3dImporter::Mesh3dImporter(EngineCore* engineCore) : engineCore(engineCore) {
	PrepareLayouts();
}

void Mesh3dImporter::PrepareLayouts() {
	VertexInputLayoutBuilder builder;
	builder.AddBinding(
		{0, 0, VertexInputRate::Vertex},
		{
			{
				"vertexPosition",
				(uint32_t)Mesh3dLayoutIndex::Position,
				Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::Position
			}
		}
	).AddBinding(
		{ 2, 0, VertexInputRate::Vertex },
		{
			{
				"vertexNormal",
				(uint32_t)Mesh3dLayoutIndex::Normal,
				Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::Normal
			}
		}
	).AddBinding(
		{ 3, 0, VertexInputRate::Vertex },
		{
			{
				"vertexTangent",
				(uint32_t)Mesh3dLayoutIndex::Tangent,
				Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::Tangent
			}
		}
	).AddBinding(
		{ 4, 0, VertexInputRate::Vertex },
		{
			{
				"vertexTexCoord0",
				(uint32_t)Mesh3dLayoutIndex::Uv0,
				Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::TexCoord0
			}
		}
	).AddBinding(
		{ 4, 0, VertexInputRate::Vertex },
		{
			{
				"vertexTexCoord1",
				(uint32_t)Mesh3dLayoutIndex::Uv1,
				Grindstone::GraphicsAPI::Format::R32G32B32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::TexCoord1
			}
		}
	);

	vertexLayout = builder.Build();
}

void Mesh3dImporter::DecrementMeshCount(ECS::Entity entity, Uuid uuid) {
	auto meshInMap = assets.find(uuid);
	if (meshInMap == assets.end()) {
		return;
	}

	Grindstone::Mesh3dAsset* mesh = &meshInMap->second;
	mesh->referenceCount -= 1;
}

void Mesh3dImporter::QueueReloadAsset(Uuid uuid) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	auto meshInMap = assets.find(uuid);
	if (meshInMap == assets.end()) {
		return;
	}

	Grindstone::Mesh3dAsset& meshAsset = meshInMap->second;
	meshAsset.assetLoadStatus = AssetLoadStatus::Reloading;
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
	std::vector<GraphicsAPI::Buffer*>& vertexBuffers
) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	std::string& assetName = mesh.name;
	auto vertexCount = header.vertexCount;
	if (header.hasVertexPositions) {
		auto positions = LoadVertexBufferVec(graphicsCore, assetName, 3, vertexCount, sourcePtr, "Positions");
		vertexBuffers.push_back(positions);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexNormals) {
		auto normals = LoadVertexBufferVec(graphicsCore, assetName, 3, vertexCount, sourcePtr, "Normals");
		vertexBuffers.push_back(normals);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexTangents) {
		auto tangents = LoadVertexBufferVec(graphicsCore, assetName, 3, vertexCount, sourcePtr, "Tangents");
		vertexBuffers.push_back(tangents);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.vertexUvSetCount >= 1) {
		auto uv0 = LoadVertexBufferVec(graphicsCore, assetName, 2, vertexCount, sourcePtr, "TexCoord0");
		vertexBuffers.push_back(uv0);
		sourcePtr += sizeof(float) * 2 * vertexCount;
	}
}

void Mesh3dImporter::LoadMeshImportIndices(
	Mesh3dAsset& mesh,
	Formats::Model::V1::Header& header,
	char*& sourcePtr,
	GraphicsAPI::Buffer*& indexBuffer
) {
	auto graphicsCore = engineCore->GetGraphicsCore();
	std::vector<uint16_t> indices;
	indices.resize(header.indexCount);
	uint64_t indexSize = header.indexCount * sizeof(uint16_t);
	memcpy(indices.data(), sourcePtr, indexSize);
	sourcePtr += indexSize;

	std::string debugName = mesh.name + " Index Buffer";
	GraphicsAPI::Buffer::CreateInfo indexBufferCreateInfo{};
	indexBufferCreateInfo.debugName = debugName.c_str();
	indexBufferCreateInfo.content = indices.data();
	indexBufferCreateInfo.bufferUsage = BufferUsage::Index;
	indexBufferCreateInfo.memoryUsage = MemUsage::GPUOnly;
	indexBufferCreateInfo.bufferSize = static_cast<uint32_t>(indices.size() * sizeof(indices[0]));
	indexBuffer = graphicsCore->CreateBuffer(indexBufferCreateInfo);
}

void* Mesh3dImporter::LoadAsset(Uuid uuid) {
	auto& meshIterator = assets.emplace(uuid, Mesh3dAsset(uuid, uuid.ToString()));
	Mesh3dAsset& meshAsset = meshIterator.first->second;

	meshAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!ImportModelFile(meshAsset)) {
		return nullptr;
	}

	return &meshAsset;
}

bool Mesh3dImporter::ImportModelFile(Mesh3dAsset& mesh) {
	Grindstone::Assets::AssetLoadBinaryResult result = engineCore->assetManager->LoadBinaryByUuid(AssetType::Texture, mesh.uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Mesh3dImporter::LoadAsset Unable to load file with id: {}", mesh.uuid.ToString());
		mesh.assetLoadStatus = AssetLoadStatus::Missing;
		return false;
	}

	char* fileContent = reinterpret_cast<char*>(result.buffer.Get());
	uint64_t fileSize = result.buffer.GetCapacity();
	mesh.name = result.displayName;

	auto graphicsCore = engineCore->GetGraphicsCore();
	if (fileSize < 3 && strncmp("GMF", fileContent, 3) != 0) {
		GPRINT_ERROR(LogSource::EngineCore, "Mesh3dImporter::LoadAsset GMF magic code wasn't matched.");
		mesh.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	Formats::Model::V1::Header header;
	if (fileSize < (3 + sizeof(header))) {
		GPRINT_ERROR(LogSource::EngineCore, "Mesh3dImporter::LoadAsset file not big enough to fit header.");
		mesh.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	char* headerPtr = fileContent + 3;
	header = *(Formats::Model::V1::Header*)headerPtr;

	char* srcPtr = headerPtr + sizeof(header);

	uint64_t totalFileExpectedSize = GetTotalFileSize(header);
	if (totalFileExpectedSize > fileSize || header.totalFileSize > fileSize) {
		GPRINT_ERROR(LogSource::EngineCore, "Mesh3dImporter::LoadAsset file not big enough to fit all contents.");
		mesh.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	std::vector<GraphicsAPI::Buffer*> vertexBuffers;
	GraphicsAPI::Buffer* indexBuffer = nullptr;

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

	for (size_t i = 0; i < mesh.submeshes.size(); ++i) {
		mesh.submeshes[i].vertexArrayObject = mesh.vertexArrayObject;
	}

	mesh.assetLoadStatus = AssetLoadStatus::Ready;
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

Grindstone::GraphicsAPI::VertexInputLayout Mesh3dImporter::vertexLayout;
