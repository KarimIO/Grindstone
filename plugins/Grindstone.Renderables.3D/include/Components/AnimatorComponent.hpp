#pragma once

#include <string>
#include <vector>

#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <EngineCore/WorldContext/WorldContextSet.hpp>

#include <Grindstone.Renderables.3D/include/Assets/AnimationClipAsset.hpp>
#include <Grindstone.Renderables.3D/include/Assets/RigAsset.hpp>

namespace Grindstone {
	namespace GraphicsAPI {
		class Buffer;
	}

	struct AnimatorComponent {
		static void Destroy(Grindstone::WorldContextSet& worldContextSet, entt::entity entity);
		AssetReference<AnimationClipAsset> animation;
		AssetReference<RigAsset> rig;
		GraphicsAPI::Buffer* skeletonMatrixBuffer = nullptr;
		
		REFLECT("Animator")
	};
}
