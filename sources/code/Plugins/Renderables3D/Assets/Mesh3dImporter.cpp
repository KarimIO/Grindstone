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
	vertexBufferCreateInfo.bufferUsage =
		GraphicsAPI::BufferUsage::TransferDst |
		GraphicsAPI::BufferUsage::TransferSrc |
		Grindstone::GraphicsAPI::BufferUsage::Vertex;
	vertexBufferCreateInfo.bufferSize = static_cast<uint32_t>(size);
	return graphicsCore->CreateBuffer(vertexBufferCreateInfo);
}

Mesh3dImporter::Mesh3dImporter(EngineCore* engineCore) : engineCore(engineCore) {
	PrepareLayouts();
}

void Mesh3dImporter::PrepareLayouts() {
	GraphicsAPI::VertexInputLayoutBuilder builder;
	builder.AddBinding(
		{0, 3 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex},
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
		{ 1, 3 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex },
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
		{ 2, 3 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex },
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
		{ 3, 2 * sizeof(float), GraphicsAPI::VertexInputRate::Vertex },
		{
			{
				"vertexTexCoord0",
				(uint32_t)Mesh3dLayoutIndex::Uv0,
				Grindstone::GraphicsAPI::Format::R32G32_SFLOAT,
				0,
				Grindstone::GraphicsAPI::AttributeUsage::TexCoord0
			}
		}
	);

	vertexLayout = builder.Build();
}

void Mesh3dImporter::OnDeleteAsset(Grindstone::Mesh3dAsset& asset) {
	GraphicsAPI::Core* graphicsCore = engineCore->GetGraphicsCore();
	graphicsCore->DeleteVertexArrayObject(asset.vertexArrayObject);
	graphicsCore->DeleteBuffer(asset.positionBuffer);
	graphicsCore->DeleteBuffer(asset.normalBuffer);
	graphicsCore->DeleteBuffer(asset.tangentBuffer);
	graphicsCore->DeleteBuffer(asset.uvBuffer);
	graphicsCore->DeleteBuffer(asset.indexBuffer);
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
		mesh.positionBuffer = positions;
		vertexBuffers.push_back(positions);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexNormals) {
		auto normals = LoadVertexBufferVec(graphicsCore, assetName, 3, vertexCount, sourcePtr, "Normals");
		mesh.normalBuffer = normals;
		vertexBuffers.push_back(normals);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.hasVertexTangents) {
		auto tangents = LoadVertexBufferVec(graphicsCore, assetName, 3, vertexCount, sourcePtr, "Tangents");
		mesh.tangentBuffer = tangents;
		vertexBuffers.push_back(tangents);
		sourcePtr += sizeof(float) * 3 * vertexCount;
	}

	if (header.vertexUvSetCount >= 1) {
		auto uv0 = LoadVertexBufferVec(graphicsCore, assetName, 2, vertexCount, sourcePtr, "TexCoord0");
		mesh.uvBuffer = uv0;
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
	indexBufferCreateInfo.bufferUsage =
		GraphicsAPI::BufferUsage::TransferDst |
		GraphicsAPI::BufferUsage::TransferSrc |
		GraphicsAPI::BufferUsage::Index;
	indexBufferCreateInfo.memoryUsage = GraphicsAPI::MemUsage::GPUOnly;
	indexBufferCreateInfo.bufferSize = static_cast<uint32_t>(indices.size() * sizeof(indices[0]));
	mesh.indexBuffer = indexBuffer = graphicsCore->CreateBuffer(indexBufferCreateInfo);
}

void* Mesh3dImporter::LoadAsset(Uuid uuid) {
	auto meshIterator = assets.emplace(uuid, Mesh3dAsset(uuid, uuid.ToString()));
	Mesh3dAsset& meshAsset = meshIterator.first->second;

	meshAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!ImportModelFile(meshAsset)) {
		return nullptr;
	}

	return &meshAsset;
}

bool Mesh3dImporter::ImportModelFile(Mesh3dAsset& mesh) {
	Grindstone::Assets::AssetLoadBinaryResult result = engineCore->assetManager->LoadBinaryByUuid(AssetType::Mesh3d, mesh.uuid);
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
		GPRINT_ERROR_V(LogSource::EngineCore, "Mesh3dImporter::LoadAsset \"{}\" with id \"{}\" doesn't start with GMF magic code.", mesh.name.c_str(), mesh.uuid.ToString());
		mesh.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	Formats::Model::V1::Header header;
	if (fileSize < (3 + sizeof(header))) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Mesh3dImporter::LoadAsset \"{}\" with id \"{}\" not big enough to fit header.", mesh.name.c_str(), mesh.uuid.ToString());
		mesh.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	char* headerPtr = fileContent + 3;
	header = *(Formats::Model::V1::Header*)headerPtr;

	char* srcPtr = headerPtr + sizeof(header);

	uint64_t totalFileExpectedSize = GetTotalFileSize(header);
	if (totalFileExpectedSize > fileSize || header.totalFileSize > fileSize) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Mesh3dImporter::LoadAsset \"{}\" with id \"{}\" not big enough to fit all contents.", mesh.name.c_str(), mesh.uuid.ToString());
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
	vaoCi.layout = vertexLayout;
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
		graphicsCore->DeleteBuffer(asset.second.positionBuffer);
		graphicsCore->DeleteBuffer(asset.second.normalBuffer);
		graphicsCore->DeleteBuffer(asset.second.tangentBuffer);
		graphicsCore->DeleteBuffer(asset.second.uvBuffer);
		graphicsCore->DeleteBuffer(asset.second.indexBuffer);
	}
	assets.clear();
}

Grindstone::GraphicsAPI::VertexInputLayout Mesh3dImporter::vertexLayout;
