#pragma once

#include "EngineCore/Assets/AssetImporter.hpp"
#include "Common/ResourcePipeline/Uuid.hpp"

namespace Grindstone {
	class AnimationClipImporter : public AssetImporter {
	public:
		virtual void Load(Uuid uuid) override;
	};
}
