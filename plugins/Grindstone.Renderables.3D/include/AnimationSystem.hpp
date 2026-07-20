#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/mat4x4.hpp>

#include <Common/Rendering/RenderViewData.hpp>
#include "EngineCore/AssetRenderer/BaseAssetRenderer.hpp"
#include "Components/MeshRendererComponent.hpp"
#include "Assets/Mesh3dAsset.hpp"

namespace Grindstone {
	void AnimateSkeletonSystem(Grindstone::WorldContextSet& worldContextSet);
}
