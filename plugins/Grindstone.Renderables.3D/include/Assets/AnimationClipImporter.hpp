#pragma once

#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/Assets/AssetImporter.hpp>
#include <Grindstone.Renderables.3D/include/Assets/AnimationClipAsset.hpp>

namespace Grindstone {
	class AnimationClipImporter : public SpecificAssetImporter<AnimationClipAsset, AssetType::AnimationClip> {
	public:
		virtual ~AnimationClipImporter() override;

		virtual void* LoadAsset(Uuid uuid) override;
		virtual void QueueReloadAsset(Uuid uuid) override;
		virtual void OnDeleteAsset(Grindstone::AnimationClipAsset& asset) override;
	};
}
