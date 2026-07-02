#pragma once

#include <string>
#include <vector>

#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderables.3D/include/Assets/AnimationClipAsset.hpp>

namespace Grindstone {
	struct AnimatorComponent {
		static void Destroy(Grindstone::WorldContextSet& worldContextSet, entt::entity entity);
		AssetReference<AnimationClipAsset> animation;
		REFLECT("Animator")
	};
}
