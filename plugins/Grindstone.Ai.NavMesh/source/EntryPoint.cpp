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

static void BakeNavMeshForAgent(rcContext* context, const Ai::NavMeshComponent& navMeshData, const Ai::NavAgentType& agentType) {
	WorldContextSet set;
	const entt::registry& registry = set.GetEntityRegistry();

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

}

static void BakeNavMesh() {
	Ai::GrindstoneRecastContext* context = new Ai::GrindstoneRecastContext();

	Ai::NavMeshComponent navMeshData{};
	std::vector<Ai::NavAgentType> agentTypes;

	for (Ai::NavAgentType& agentType : agentTypes) {
		// Get data from the below step and cache it
		BakeNavMeshForAgent(context, navMeshData, agentType);
	}
}

static bool CreateNavMeshFromData(
	rcContext* context,
	const Ai::NavMeshComponent& navMeshData,
	Ai::NavAgentComponent& navAgent,
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
	params.cs = navMeshData.cellSize;
	params.ch = navMeshData.cellHeight;
	params.buildBvTree = true;

	if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Could not build Detour navmesh.");
		return false;
	}

	dtNavMesh* navMesh = dtAllocNavMesh();
	if (!navMesh) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Could not create Detour navmesh");
		dtFree(navData);
		return false;
	}

	dtStatus status;

	status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status)) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Could not init Detour navmesh");
		dtFree(navData);
		dtFreeNavMesh(navMesh);
		return false;
	}


	dtNavMeshQuery* query = dtAllocNavMeshQuery();
	status = query->init(navMesh, 2048);
	if (dtStatusFailed(status)) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "Could not init Detour navmesh query");
		dtFreeNavMeshQuery(query);
		return false;
	}

	dtFreeNavMeshQuery(query);
	dtFreeNavMesh(navMesh);
}

void* recastAllocFn(size_t size, rcAllocHint hint) {
	return Grindstone::Memory::AllocatorCore::AllocateRaw(size, 1, "Recast Memory");
}

void* detourAllocFn(size_t size, dtAllocHint hint) {
	return Grindstone::Memory::AllocatorCore::AllocateRaw(size, 1, "Detour Memory");
}

void recastFreeFn(void* ptr) {
	Grindstone::Memory::AllocatorCore::FreeWithoutDestructor(ptr);
}

extern "C" {
	AI_NAVMESH_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::EngineCore::SetInstance(*pluginInterface->GetEngineCore());

		rcAllocSetCustom(recastAllocFn, recastFreeFn);
		dtAllocSetCustom(detourAllocFn, recastFreeFn);

		pluginInterface->RegisterComponent<Grindstone::Ai::NavMeshComponent>();
		pluginInterface->RegisterComponent<Grindstone::Ai::NavAgentComponent>();
		pluginInterface->RegisterComponent<Grindstone::Ai::OffNavMeshConnectionComponent>();
	}

	AI_NAVMESH_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		pluginInterface->UnregisterComponent<Grindstone::Ai::OffNavMeshConnectionComponent>();
		pluginInterface->UnregisterComponent<Grindstone::Ai::NavAgentComponent>();
		pluginInterface->UnregisterComponent<Grindstone::Ai::NavMeshComponent>();
	}
}
