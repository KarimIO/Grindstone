#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

#include <Grindstone.Physics.Bullet/include/Components/ColliderComponent.hpp>

#include <Grindstone.Ai.NavMesh/include/pch.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshSystem.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshBuildContext.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshWorldContext.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavMeshComponent.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavAgentComponent.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/OffNavMeshConnectionComponent.hpp>

#include <Editor/PluginSystem/EditorPluginInterface.hpp>

#include <Recast.h>
#include <RecastAlloc.h>

#include <DetourAlloc.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>

using namespace Grindstone;

struct OffMeshConnectionDataArrays {
	// Two vertices per connection, each of size 3 floats
	float* vertexPositions = nullptr;

	// One float per connection
	float* radii = nullptr;

	Ai::OffNavMeshConnectionComponent::Direction* directions = nullptr;
	Ai::NavAreaId* areas = nullptr;
	Ai::NavAreaFlags* flags = nullptr;
	unsigned int* userIds = nullptr;

	int connectionCount = 0;
};

static glm::vec3 CalculateMinimumBounds(glm::vec3 position, glm::vec3 boundMin) {
	return glm::vec3(
		glm::min(position.x, boundMin.x),
		glm::min(position.y, boundMin.y),
		glm::min(position.z, boundMin.z)
	);
}

static glm::vec3 CalculateMaximumBounds(glm::vec3 position, glm::vec3 boundMax) {
	return glm::vec3(
		glm::max(position.x, boundMax.x),
		glm::max(position.y, boundMax.y),
		glm::max(position.z, boundMax.z)
	);
}

static void PopulateGeometry(
	const entt::registry& registry,
	std::vector<glm::vec3>& vertexPositions,
	std::vector<int>& triangles,
	glm::vec3& boundMin,
	glm::vec3& boundMax
) {
	int index = 0;

	{
		auto view = registry.view<entt::entity, const TransformComponent, const Physics::BoxColliderComponent>();
		view.each(
			[&](
				entt::entity entity,
				const TransformComponent& transformComponent,
				const Physics::BoxColliderComponent& boxComponent
			) {
				const btBoxShape* box = static_cast<const btBoxShape*>(boxComponent.collisionShape.Get());
				box->getHalfExtentsWithMargin();
				btVector3 halfExtents = box->getHalfExtentsWithMargin();
				glm::mat4 transformMatrix = transformComponent.GetWorldTransformMatrix(entity, registry);

				glm::vec4 v[8] = {
					{ -halfExtents.x(), -halfExtents.y(), -halfExtents.z(), 1.0f },
					{  halfExtents.x(), -halfExtents.y(), -halfExtents.z(), 1.0f },
					{  halfExtents.x(),  halfExtents.y(), -halfExtents.z(), 1.0f },
					{ -halfExtents.x(),  halfExtents.y(), -halfExtents.z(), 1.0f },
					{ -halfExtents.x(), -halfExtents.y(),  halfExtents.z(), 1.0f },
					{  halfExtents.x(), -halfExtents.y(),  halfExtents.z(), 1.0f },
					{  halfExtents.x(),  halfExtents.y(),  halfExtents.z(), 1.0f },
					{ -halfExtents.x(),  halfExtents.y(),  halfExtents.z(), 1.0f },
				};

				for (int i = 0; i < 8; ++i) {
					glm::vec3 position = transformMatrix * v[i];
					vertexPositions.emplace_back(glm::vec3(position.x, position.y, position.z));

					boundMin = CalculateMinimumBounds(position, boundMin);
					boundMax = CalculateMaximumBounds(position, boundMax);
				}

				// Define triangles (two per face)
				int indices[36] = {
					0,1,2, 2,3,0,  // -Z
					1,5,6, 6,2,1,  // +X
					5,4,7, 7,6,5,  // +Z
					4,0,3, 3,7,4,  // -X
					3,2,6, 6,7,3,  // +Y (top)
					4,5,1, 1,0,4   // -Y (bottom)
				};

				for (int i = 0; i < 36; ++i) {
					triangles.emplace_back(index + indices[i]);
				}

				index += 8;
			}
		);
	}
}

static rcPolyMesh* BakeNavMeshForAgent(const entt::registry& registry, rcContext* context, const Ai::NavMeshComponent& navMeshData, const Ai::NavAgentType& agentType) {
	glm::vec3 bmin, bmax;
	std::vector<glm::vec3> verts;
	std::vector<int> tris;

	PopulateGeometry(registry, verts, tris, bmin, bmax);

	int borderSize = 1;
	int tileSize = 4;
	float edgeMaxLen = 12.0f;
	float edgeMaxError = 1.3f;
	int regionMinSize = 8.0f;
	int regionMergeSize = 20.0f;
	int vertsPerPoly = 6;
	float detailSampleDist = 6.0f;
	float detailSampleMaxError = 1.0f;

	rcConfig config{
		.tileSize = tileSize,
		.borderSize = borderSize,
		.cs = navMeshData.cellSize,
		.ch = navMeshData.cellHeight,
		.bmin = { bmin.x, bmin.y, bmin.z },
		.bmax = { bmax.x, bmax.y, bmax.z },
		.walkableSlopeAngle = agentType.maxSlope,
		.walkableHeight = (int)ceilf(agentType.height / navMeshData.cellHeight),
		.walkableClimb = (int)floorf(agentType.stepHeight / navMeshData.cellHeight),
		.walkableRadius = (int)ceilf(agentType.radius / navMeshData.cellSize),
		.maxEdgeLen = (int)(edgeMaxLen / navMeshData.cellSize),
		.maxSimplificationError = edgeMaxError,
		.minRegionArea = (int)rcSqr(regionMinSize),
		.mergeRegionArea = (int)rcSqr(regionMergeSize),
		.maxVertsPerPoly = 3,
		.detailSampleDist = detailSampleDist < 0.9f ? 0 : navMeshData.cellSize * detailSampleDist,
		.detailSampleMaxError = navMeshData.cellHeight * detailSampleMaxError,
	};

	rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);

	int numVerts = verts.size() * 3u;
	float* vertsPtr = reinterpret_cast<float*>(verts.data());
	int numTris = tris.size() * 3u;
	std::vector<unsigned char> areas;

	// Create and Build Heightfield

	rcHeightfield* heightfield = rcAllocHeightfield();
	rcCreateHeightfield(context, *heightfield, config.width, config.height, config.bmin, config.bmax, config.cs, config.ch);
	rcMarkWalkableTriangles(context, config.walkableSlopeAngle, vertsPtr, numVerts, tris.data(), numTris, areas.data());
	rcRasterizeTriangles(context, vertsPtr, numVerts, tris.data(), areas.data(), numTris, *heightfield, config.walkableClimb);

	// Apply Filters

	rcFilterLowHangingWalkableObstacles(context, config.walkableClimb, *heightfield);
	rcFilterLedgeSpans(context, config.walkableHeight, config.walkableClimb, *heightfield);
	rcFilterWalkableLowHeightSpans(context, config.walkableHeight, *heightfield);

	// Build Compact Heightfield

	rcCompactHeightfield* compactHeightfield = rcAllocCompactHeightfield();
	rcBuildCompactHeightfield(context, config.walkableHeight, config.walkableClimb, *heightfield, *compactHeightfield);
	rcFreeHeightField(heightfield);

	// Build Regions and Contours

	rcBuildDistanceField(context, *compactHeightfield);
	rcBuildRegions(context, *compactHeightfield, config.borderSize, config.minRegionArea, config.mergeRegionArea);

	rcContourSet* contourSet = rcAllocContourSet();
	rcBuildContours(context, *compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *contourSet, RC_CONTOUR_TESS_WALL_EDGES);

	// Build Polygon Mesh

	rcPolyMesh* polyMesh = rcAllocPolyMesh();
	rcBuildPolyMesh(context, *contourSet, config.maxVertsPerPoly, *polyMesh);
	rcFreeContourSet(contourSet);  // Contour set no longer needed

	// Build Detail Mesh(Optional)

	rcPolyMeshDetail* polyMeshDetail = rcAllocPolyMeshDetail();
	rcBuildPolyMeshDetail(context, *polyMesh, *compactHeightfield, config.detailSampleDist, config.detailSampleMaxError, *polyMeshDetail);
	rcFreeCompactHeightfield(compactHeightfield);  // Compact heightfield no longer needed

	return polyMesh;
}

static dtNavMesh* CreateNavMeshFromData(
	rcContext* context,
	const Ai::NavMeshComponent& navMeshData,
	Ai::NavAgentType& agentType,
	const OffMeshConnectionDataArrays& offMeshConnections,
	rcPolyMesh* polyMesh,
	rcPolyMeshDetail* polyMeshDetail
) {
	unsigned char* navData = 0;
	int navDataSize = 0;

	dtNavMeshCreateParams params{};
	params.verts = polyMesh->verts;
	params.vertCount = polyMesh->nverts;
	params.polys = polyMesh->polys;
	params.polyAreas = polyMesh->areas;
	params.polyFlags = polyMesh->flags;
	params.polyCount = polyMesh->npolys;
	params.nvp = polyMesh->nvp;
	params.detailMeshes = polyMeshDetail->meshes;
	params.detailVerts = polyMeshDetail->verts;
	params.detailVertsCount = polyMeshDetail->nverts;
	params.detailTris = polyMeshDetail->tris;
	params.detailTriCount = polyMeshDetail->ntris;
	params.offMeshConVerts = offMeshConnections.vertexPositions;
	params.offMeshConRad = offMeshConnections.radii;
	params.offMeshConDir = reinterpret_cast<unsigned char*>(offMeshConnections.directions);
	params.offMeshConAreas = static_cast<unsigned char*>(offMeshConnections.areas);
	params.offMeshConFlags = static_cast<unsigned short*>(offMeshConnections.flags);
	params.offMeshConUserID = offMeshConnections.userIds;
	params.offMeshConCount = offMeshConnections.connectionCount;
	params.walkableHeight = agentType.height;
	params.walkableRadius = agentType.radius;
	params.walkableClimb = agentType.stepHeight;
	rcVcopy(params.bmin, polyMesh->bmin);
	rcVcopy(params.bmax, polyMesh->bmax);
	params.cs = polyMesh->cs; // navMeshData.cellSize;
	params.ch = polyMesh->ch; // navMeshData.cellHeight;
	params.buildBvTree = true;

	if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Could not build Detour navmesh.");
		return nullptr;
	}

	dtNavMesh* navMesh = dtAllocNavMesh();
	if (!navMesh) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Could not create Detour navmesh");
		dtFree(navData);
		return nullptr;
	}

	dtStatus status;

	status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status)) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Could not init Detour navmesh");
		dtFree(navData);
		dtFreeNavMesh(navMesh);
		return nullptr;
	}

	return navMesh;
}

static void* RecastAllocFn(size_t size, rcAllocHint hint) {
	return Grindstone::Memory::AllocatorCore::AllocateRaw(size, 1, "Recast Memory");
}

static void* DetourAllocFn(size_t size, dtAllocHint hint) {
	return Grindstone::Memory::AllocatorCore::AllocateRaw(size, 1, "Detour Memory");
}

static void RecastFreeFn(void* ptr) {
	Grindstone::Memory::AllocatorCore::FreeWithoutDestructor(ptr);
}

static const uint32_t NAVMESHSET_MAGIC = 'G' << 24 | 'N' << 16 | 'A' << 8 | 'V';
static const uint32_t NAVMESHSET_VERSION = 1;

struct NavMeshSetHeader {
	uint32_t magic;
	uint32_t version;
	uint32_t numTiles;
	dtNavMeshParams params;
};

struct NavMeshTileHeader {
	dtTileRef tileRef;
	uint32_t dataSize;
};

static void WriteNavmesh(const std::filesystem::path& path, const dtNavMesh* mesh) {
	if (!mesh) {
		return;
	}

	std::ofstream fstream(path, std::ios::binary);
	if (fstream.fail()) {
		return;
	}

	NavMeshSetHeader header{
		.magic = NAVMESHSET_MAGIC,
		.version = NAVMESHSET_VERSION,
		.numTiles = 0,
	};

	for (int i = 0; i < mesh->getMaxTiles(); ++i) {
		const dtMeshTile* tile = mesh->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) {
			continue;
		}

		header.numTiles++;
	}

	memcpy(&header.params, mesh->getParams(), sizeof(dtNavMeshParams));
	fstream.write(reinterpret_cast<const char*>(&header), sizeof(NavMeshSetHeader));

	// Store tiles.
	for (int i = 0; i < mesh->getMaxTiles(); ++i) {
		const dtMeshTile* tile = mesh->getTile(i);
		if (!tile || !tile->header || !tile->dataSize) {
			continue;
		}

		NavMeshTileHeader tileHeader{
			.tileRef = mesh->getTileRef(tile),
			.dataSize = static_cast<uint32_t>(tile->dataSize)
		};

		fstream.write(reinterpret_cast<const char*>(&tileHeader), sizeof(tileHeader));
		fstream.write(reinterpret_cast<const char*>(tile->data), tile->dataSize);
	}
}

static dtNavMesh* LoadNavmesh(const char* path) {
	FILE* fp = fopen(path, "rb");
	if (!fp) {
		return nullptr;
	}

	// Read header.
	NavMeshSetHeader header;
	size_t readLen = fread(&header, sizeof(NavMeshSetHeader), 1, fp);
	if (readLen != 1 || header.magic != NAVMESHSET_MAGIC || header.version != NAVMESHSET_VERSION) {
		fclose(fp);
		return nullptr;
	}

	dtNavMesh* mesh = dtAllocNavMesh();
	if (!mesh) {
		fclose(fp);
		return nullptr;
	}

	dtStatus status = mesh->init(&header.params);
	if (dtStatusFailed(status)) {
		fclose(fp);
		return nullptr;
	}

	// Read tiles.
	for (int i = 0; i < header.numTiles; ++i) {
		NavMeshTileHeader tileHeader;
		readLen = fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (readLen != 1) {
			fclose(fp);
			return nullptr;
		}

		if (!tileHeader.tileRef || !tileHeader.dataSize) {
			break;
		}

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) {
			break;
		}

		memset(data, 0, tileHeader.dataSize);
		readLen = fread(data, tileHeader.dataSize, 1, fp);
		if (readLen != 1) {
			dtFree(data);
			fclose(fp);
			return nullptr;
		}

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	fclose(fp);

	return mesh;
}

static void MenuItemGenerateNavMesh() {
	GPRINT_INFO(LogSource::EngineCore, "Generating Navigation Mesh...");

	Ai::GrindstoneRecastContext* context = Grindstone::Memory::AllocatorCore::Allocate<Ai::GrindstoneRecastContext>();
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::WorldContextSet* worldContextSet = engineCore.GetWorldContextManager()->GetActiveWorldContextSet();

	if (worldContextSet == nullptr) {
		GPRINT_INFO(LogSource::EngineCore, "Failed to generate Navigation Mesh - no active WorldContextSet.");
		return;
	}

	Grindstone::Ai::NavMeshComponent* foundNavMeshComp = nullptr;
	entt::registry& registry = worldContextSet->GetEntityRegistry();
	registry.view<Grindstone::Ai::NavMeshComponent>().each(
		[&foundNavMeshComp](
			Grindstone::Ai::NavMeshComponent& navMesh
		) {
			foundNavMeshComp = &navMesh;
		}
	);
	if (foundNavMeshComp == nullptr) {
		GPRINT_INFO(LogSource::EngineCore, "Failed to generate Navigation Mesh - no NavigationMeshComponent found.");
		return;
	}

	Ai::NavMeshComponent navMeshData{};
	std::vector<Ai::NavAgentType> agentTypes;
	agentTypes.emplace_back(Ai::NavAgentType{
		.radius = 0.5,
		.height = 2.0f,
		.stepHeight = 0.1f,
		.maxSlope = 30.0f,
		.dropHeight = 1.0f,
		.jumpDistance = 1.0f,
	});

	std::vector<rcPolyMesh*> meshes;

	for (Ai::NavAgentType& agentType : agentTypes) {
		// Get data from the below step and cache it
		rcPolyMesh* mesh = BakeNavMeshForAgent(registry, context, navMeshData, agentType);
		meshes.emplace_back(mesh);
		break;
	}

	std::filesystem::path path = "";
	WriteNavmesh(path, meshes[0]);

	Grindstone::Memory::AllocatorCore::Free(context);

	GPRINT_INFO(LogSource::EngineCore, "Generated Navigation Mesh.");
}

extern "C" {
	AI_NAVMESH_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::EngineCore::SetInstance(*pluginInterface->GetEngineCore());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		rcAllocSetCustom(RecastAllocFn, RecastFreeFn);
		dtAllocSetCustom(DetourAllocFn, RecastFreeFn);

		pluginInterface->RegisterComponent<Grindstone::Ai::NavMeshComponent>();
		pluginInterface->RegisterComponent<Grindstone::Ai::NavAgentComponent>();
		pluginInterface->RegisterComponent<Grindstone::Ai::OffNavMeshConnectionComponent>();

		Grindstone::Plugins::EditorPluginInterface* editorInterface = static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());
		if (editorInterface != nullptr) {
			editorInterface->RegisterMenuItem("Build/Generate Navigation Mesh", MenuItemGenerateNavMesh);
		}
	}

	AI_NAVMESH_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Grindstone::Plugins::EditorPluginInterface* editorInterface = static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());
		if (editorInterface != nullptr) {
			editorInterface->DeregisterMenuItem("Build/Generate Navigation Mesh");
		}

		pluginInterface->UnregisterComponent<Grindstone::Ai::OffNavMeshConnectionComponent>();
		pluginInterface->UnregisterComponent<Grindstone::Ai::NavAgentComponent>();
		pluginInterface->UnregisterComponent<Grindstone::Ai::NavMeshComponent>();
	}
}
