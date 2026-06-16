#pragma once

#include <string>
#include <vector>

#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include "../Assets/Mesh3dAsset.hpp"

namespace Grindstone {
	struct MeshComponent {
		static void Destroy(Grindstone::WorldContextSet& worldContextSet, entt::entity entity);
		AssetReference<Mesh3dAsset> mesh;
		REFLECT("Mesh")
	};
}
