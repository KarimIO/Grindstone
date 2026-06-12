#include <EngineCore/PluginSystem/Interface.hpp>
#include <EngineCore/Utils/MemoryAllocator.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>

#include <Grindstone.Physics.Jolt/include/Components/ColliderComponent.hpp>

#include <Grindstone.Ai.NavMesh/include/pch.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshAgentSystem.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshBuildContext.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshWorldContext.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavMeshComponent.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/NavAgentComponent.hpp>
#include <Grindstone.Ai.NavMesh/include/Components/OffNavMeshConnectionComponent.hpp>
#include <Grindstone.Ai.NavMesh/include/NavMeshDebugRenderer.hpp>

#include <Editor/PluginSystem/EditorPluginInterface.hpp>

#include <Recast.h>
#include <RecastAlloc.h>

#include <DetourDebugDraw.h>
#include <DetourAlloc.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourNavMeshQuery.h>

using namespace Grindstone;

Grindstone::Ai::NavMeshDebugRenderer* debugRenderer = nullptr;

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
	constexpr float inf = std::numeric_limits<float>().infinity();
	constexpr float ninf = std::numeric_limits<float>().lowest();
	boundMin = { inf, inf, inf };
	boundMax = { ninf, ninf, ninf };
	int index = 0;

	{
		auto view = registry.view<entt::entity, const TransformComponent, const Physics::BoxColliderComponent>();
		view.each(
			[&](
				entt::entity entity,
				const TransformComponent& transformComponent,
				const Physics::BoxColliderComponent& boxComponent
			) {
				glm::vec3 halfExtents = boxComponent.GetSize() / 2.0f;
				glm::mat4 transformMatrix = transformComponent.GetWorldTransformMatrix(entity, registry);
				glm::vec3 scale = transformComponent.scale;

				glm::vec4 v[8] = {
					{ -halfExtents.x, -halfExtents.y, -halfExtents.z, 1.0f },
					{  halfExtents.x, -halfExtents.y, -halfExtents.z, 1.0f },
					{  halfExtents.x,  halfExtents.y, -halfExtents.z, 1.0f },
					{ -halfExtents.x,  halfExtents.y, -halfExtents.z, 1.0f },
					{ -halfExtents.x, -halfExtents.y,  halfExtents.z, 1.0f },
					{  halfExtents.x, -halfExtents.y,  halfExtents.z, 1.0f },
					{  halfExtents.x,  halfExtents.y,  halfExtents.z, 1.0f },
					{ -halfExtents.x,  halfExtents.y,  halfExtents.z, 1.0f },
				};

				for (int i = 0; i < 8; ++i) {
					glm::vec3 position = (transformMatrix * v[i]);
					position.x /= scale.x;
					position.y /= scale.y;
					position.z /= scale.z;
					vertexPositions.emplace_back(glm::vec3(position.x, position.y, position.z));

					boundMin = CalculateMinimumBounds(position, boundMin);
					boundMax = CalculateMaximumBounds(position, boundMax);
				}

				// Define triangles (two per face)
				int indices[36] = {
					0,1,2, 2,3,0,   // -Z
					1,5,6, 6,2,1,   // +X
					5,4,7, 7,6,5,   // +Z
					4,0,3, 3,7,4,   // -X
					3,7,6, 6,2,3,   // +Y (top)
					4,1,5, 5,0,4    // -Y (bottom)
				};

				for (int i = 0; i < 36; ++i) {
					triangles.emplace_back(index + indices[i]);
				}

				index += 8;
			}
		);
	}
}

static std::pair<rcPolyMesh*, rcPolyMeshDetail*> BakeNavMeshForAgent(const entt::registry& registry, rcContext* context, const Ai::NavMeshComponent& navMeshData, const Ai::NavAgentType& agentType) {
	glm::vec3 bmin, bmax;
	std::vector<glm::vec3> verts;
	std::vector<int> tris;

	PopulateGeometry(registry, verts, tris, bmin, bmax);

	bmin -= glm::vec3(2.0f, 2.0f, 2.0f);
	bmax += glm::vec3(2.0f, 2.0f, 2.0f);

	duDebugDrawBoxWire(
		debugRenderer,
		bmin[0],
		bmin[1],
		bmin[2],
		bmax[0],
		bmax[1],
		bmax[2],
		duRGBA(255, 255, 255, 128),
		1.0f
	);

	int borderSize = 0;
	int tileSize = 4;
	float edgeMaxLen = 12.0f;
	float edgeMaxError = 1.3f;
	int regionMinSize = 8.0f;
	int regionMergeSize = 20.0f;
	int vertsPerPoly = 3;
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
		.maxVertsPerPoly = vertsPerPoly,
		.detailSampleDist = detailSampleDist < 0.9f ? 0 : navMeshData.cellSize * detailSampleDist,
		.detailSampleMaxError = navMeshData.cellHeight * detailSampleMaxError,
	};

	rcCalcGridSize(config.bmin, config.bmax, config.cs, &config.width, &config.height);

	int numVerts = verts.size();
	float* vertsPtr = reinterpret_cast<float*>(verts.data());
	int numTris = tris.size() / 3;
	std::vector<unsigned char> areas;
	areas.resize(numTris);

	// Create and Build Heightfield

	rcHeightfield* heightfield = rcAllocHeightfield();
	if (heightfield == nullptr) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to allocate memory for heightfield.");
		return { nullptr, nullptr };
	}

	if (!rcCreateHeightfield(context, *heightfield, config.width, config.height, config.bmin, config.bmax, config.cs, config.ch)) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to create heightfield.");
		return { nullptr, nullptr };
	}

	rcMarkWalkableTriangles(context, config.walkableSlopeAngle, vertsPtr, numVerts, tris.data(), numTris, areas.data());
	if (!rcRasterizeTriangles(context, vertsPtr, numVerts, tris.data(), areas.data(), numTris, *heightfield, config.walkableClimb)) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to rasterize triangles.");
		return { nullptr, nullptr };
	}

	// Apply Filters

	rcFilterLowHangingWalkableObstacles(context, config.walkableClimb, *heightfield);
	rcFilterLedgeSpans(context, config.walkableHeight, config.walkableClimb, *heightfield);
	rcFilterWalkableLowHeightSpans(context, config.walkableHeight, *heightfield);

	// Build Compact Heightfield

	rcCompactHeightfield* compactHeightfield = rcAllocCompactHeightfield();
	if (compactHeightfield == nullptr) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to allocate memory for compact heightfield.");
		return { nullptr, nullptr };
	}

	if (!rcBuildCompactHeightfield(context, config.walkableHeight, config.walkableClimb, *heightfield, *compactHeightfield)) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to build compact heightfield.");
		return { nullptr, nullptr };
	}

	rcFreeHeightField(heightfield);

	// Build Regions and Contours

	if (!rcBuildDistanceField(context, *compactHeightfield)) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to build distance field.");
		return { nullptr, nullptr };
	}

	if (!rcBuildRegions(context, *compactHeightfield, config.borderSize, config.minRegionArea, config.mergeRegionArea)) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to build regions.");
		return { nullptr, nullptr };
	}

	rcContourSet* contourSet = rcAllocContourSet();
	if (contourSet == nullptr) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to allocate memory for contour set.");
		return { nullptr, nullptr };
	}

	if (!rcBuildContours(context, *compactHeightfield, config.maxSimplificationError, config.maxEdgeLen, *contourSet, RC_CONTOUR_TESS_WALL_EDGES)) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to build contours.");
		return { nullptr, nullptr };
	}

	// Build Polygon Mesh

	rcPolyMesh* polyMesh = rcAllocPolyMesh();
	if (polyMesh == nullptr) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to allocate memory for poly mesh.");
		return { nullptr, nullptr };
	}

	if (!rcBuildPolyMesh(context, *contourSet, config.maxVertsPerPoly, *polyMesh)) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to build poly mesh.");
		return { nullptr, nullptr };
	}

	rcFreeContourSet(contourSet);

	// Build Detail Mesh(Optional)

	rcPolyMeshDetail* polyMeshDetail = rcAllocPolyMeshDetail();
	if (polyMeshDetail == nullptr) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to allocate memory for poly mesh detail.");
		return { polyMesh, nullptr };
	}

	if (!rcBuildPolyMeshDetail(context, *polyMesh, *compactHeightfield, config.detailSampleDist, config.detailSampleMaxError, *polyMeshDetail)) {
		GPRINT_ERROR(LogSource::EngineCore, "BakeNavMeshForAgent: Unable to build poly mesh detail.");
		return { polyMesh, nullptr };
	}

	rcFreeCompactHeightfield(compactHeightfield);

	return { polyMesh, polyMeshDetail };
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

	dtNavMeshCreateParams params{
		.verts = polyMesh->verts,
		.vertCount = polyMesh->nverts,
		.polys = polyMesh->polys,
		.polyFlags = polyMesh->flags,
		.polyAreas = polyMesh->areas,
		.polyCount = polyMesh->npolys,
		.nvp = polyMesh->nvp,
		.offMeshConVerts = offMeshConnections.vertexPositions,
		.offMeshConRad = offMeshConnections.radii,
		.offMeshConFlags = static_cast<unsigned short*>(offMeshConnections.flags),
		.offMeshConAreas = static_cast<unsigned char*>(offMeshConnections.areas),
		.offMeshConDir = reinterpret_cast<unsigned char*>(offMeshConnections.directions),
		.offMeshConUserID = offMeshConnections.userIds,
		.offMeshConCount = offMeshConnections.connectionCount,
		.walkableHeight = agentType.height,
		.walkableRadius = agentType.radius,
		.walkableClimb = agentType.stepHeight,
		.cs = polyMesh->cs, // navMeshData.cellSize,
		.ch = polyMesh->ch, // navMeshData.cellHeight
		.buildBvTree = true
	};

	if (polyMeshDetail) {
		params.detailMeshes = polyMeshDetail->meshes;
		params.detailVerts = polyMeshDetail->verts;
		params.detailVertsCount = polyMeshDetail->nverts;
		params.detailTris = polyMeshDetail->tris;
		params.detailTriCount = polyMeshDetail->ntris;
	}

	rcVcopy(params.bmin, polyMesh->bmin);
	rcVcopy(params.bmax, polyMesh->bmax);

	if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "BakeNavMeshForAgent: Could not build Detour navmesh.");
		return nullptr;
	}

	dtNavMesh* navMesh = dtAllocNavMesh();
	if (!navMesh) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "BakeNavMeshForAgent: Could not create Detour navmesh");
		dtFree(navData);
		return nullptr;
	}

	dtStatus status;

	status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
	if (dtStatusFailed(status)) {
		GPRINT_ERROR(Grindstone::LogSource::EngineCore, "BakeNavMeshForAgent: Could not init Detour navmesh");
		dtFree(navData);
		dtFreeNavMesh(navMesh);
		return nullptr;
	}

	return navMesh;
}

static void* RecastAllocFn(size_t size, rcAllocHint hint) {
	return Grindstone::Memory::AllocatorCore::AllocateRaw(size, 16, "Recast Memory");
}

static void* DetourAllocFn(size_t size, dtAllocHint hint) {
	return Grindstone::Memory::AllocatorCore::AllocateRaw(size, 16, "Detour Memory");
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

static void WriteNavmesh(const std::filesystem::path& path, const std::vector<dtNavMesh*>& meshes) {
	if (meshes.empty()) {
		return;
	}

	GS_ASSERT(meshes.size() == 1); // TODO: Support more than one nav agent type.

	std::ofstream fstream(path, std::ios::binary);
	if (fstream.fail()) {
		return;
	}

	NavMeshSetHeader header{
		.magic = NAVMESHSET_MAGIC,
		.version = NAVMESHSET_VERSION,
		.numTiles = 0,
	};

	const dtNavMesh* mesh = meshes[0];
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

static dtNavMesh* LoadNavmesh(const std::filesystem::path& path) {
	std::string pathStr = path.string();
	FILE* fp = fopen(pathStr.c_str(), "rb");
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

	debugRenderer->Clear();

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

	OffMeshConnectionDataArrays offMeshConnections{};

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

	std::vector<dtNavMesh*> meshes;

	for (Ai::NavAgentType& agentType : agentTypes) {
		// Get data from the below step and cache it
		auto [mesh, detailMesh] = BakeNavMeshForAgent(registry, context, navMeshData, agentType);

		if (mesh != nullptr) {
			dtNavMesh* dtNavMesh = CreateNavMeshFromData(
				context,
				navMeshData,
				agentType,
				offMeshConnections,
				mesh,
				detailMesh
			);

			meshes.emplace_back(dtNavMesh);

			duDebugDrawPolyMesh(debugRenderer, *mesh);

			rcFreePolyMesh(mesh);
		}

		if (detailMesh != nullptr) {
			rcFreePolyMeshDetail(detailMesh);
		}

		break;
	}

	std::filesystem::path path = "C:\\Work\\Navmesh.gnav";
	WriteNavmesh(path, meshes);

	debugRenderer->BuildVertexBuffers();

	Grindstone::Memory::AllocatorCore::Free(context);

	GPRINT_INFO(LogSource::EngineCore, "Generated Navigation Mesh.");
}

static void MenuItemLoadNavMesh() {
	GPRINT_INFO(LogSource::EngineCore, "Loading Navigation Mesh...");

	Ai::GrindstoneRecastContext* context = Grindstone::Memory::AllocatorCore::Allocate<Ai::GrindstoneRecastContext>();
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::WorldContextSet* worldContextSet = engineCore.GetWorldContextManager()->GetActiveWorldContextSet();

	if (worldContextSet == nullptr) {
		GPRINT_INFO(LogSource::EngineCore, "Failed to load Navigation Mesh - no active WorldContextSet.");
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
		GPRINT_INFO(LogSource::EngineCore, "Failed to load Navigation Mesh - no NavigationMeshComponent found.");
		return;
	}

	std::filesystem::path path = "C:\\Work\\Navmesh.gnav";

	if (!std::filesystem::exists(path)) {
		GPRINT_INFO(LogSource::EngineCore, "Failed to load Navigation Mesh - no NavigationMeshComponent found.");
		return;
	}

	dtNavMesh* navMesh = LoadNavmesh(path);
	foundNavMeshComp->navMesh = navMesh;

	debugRenderer->Clear();
	duDebugDrawNavMesh(debugRenderer, *navMesh, DU_DRAWNAVMESH_OFFMESHCONS);
	debugRenderer->BuildVertexBuffers();

	GPRINT_INFO(LogSource::EngineCore, "Loaded Navigation Mesh.");
}

extern "C" {
	AI_NAVMESH_EXPORT void InitializeModule(Plugins::Interface* pluginInterface) {
		Grindstone::HashedString::SetHashMap(pluginInterface->GetHashedStringMap());
		Grindstone::Logger::SetLoggerState(pluginInterface->GetLoggerState());
		Grindstone::EngineCore::SetInstance(*pluginInterface->GetEngineCore());
		Grindstone::Memory::AllocatorCore::SetAllocatorState(pluginInterface->GetAllocatorState());

		debugRenderer = Grindstone::Memory::AllocatorCore::Allocate<Grindstone::Ai::NavMeshDebugRenderer>();
		debugRenderer->Initialize();

		rcAllocSetCustom(RecastAllocFn, RecastFreeFn);
		dtAllocSetCustom(DetourAllocFn, RecastFreeFn);

		pluginInterface->RegisterComponent<Grindstone::Ai::NavMeshComponent>();
		pluginInterface->RegisterComponent<Grindstone::Ai::NavAgentComponent>();
		pluginInterface->RegisterComponent<Grindstone::Ai::OffNavMeshConnectionComponent>();

		Grindstone::Plugins::EditorPluginInterface* editorInterface = static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());
		if (editorInterface != nullptr) {
			editorInterface->RegisterGizmoPass(
				[](
					Grindstone::Renderer::RenderGraphBuilder& renderGraphBuilder,
					Grindstone::Renderer::RenderGraphBuilderResourceRef colorImageRef,
					Grindstone::Renderer::RenderGraphBuilderResourceRef depthImageRef
				) -> Grindstone::Renderer::RenderGraphBuilderResourceRef {
					return debugRenderer->DrawRenderPass(renderGraphBuilder, colorImageRef, depthImageRef);
				}
			);
			editorInterface->RegisterMenuItem("Build/Generate Navigation Mesh", MenuItemGenerateNavMesh);
			editorInterface->RegisterMenuItem("Build/Load Navigation Mesh", MenuItemLoadNavMesh);
		}
	}

	AI_NAVMESH_EXPORT void ReleaseModule(Plugins::Interface* pluginInterface) {
		Grindstone::Plugins::EditorPluginInterface* editorInterface = static_cast<Grindstone::Plugins::EditorPluginInterface*>(pluginInterface->GetEditorInterface());
		if (editorInterface != nullptr) {
			editorInterface->DeregisterMenuItem("Build/Generate Navigation Mesh");
			editorInterface->DeregisterMenuItem("Build/Load Navigation Mesh");
		}

		if (debugRenderer != nullptr) {
			Grindstone::Memory::AllocatorCore::Free(debugRenderer);
		}

		pluginInterface->UnregisterComponent<Grindstone::Ai::OffNavMeshConnectionComponent>();
		pluginInterface->UnregisterComponent<Grindstone::Ai::NavAgentComponent>();
		pluginInterface->UnregisterComponent<Grindstone::Ai::NavMeshComponent>();
	}
}
