#pragma once

#include <Common/ResourcePipeline/Uuid.hpp>
#include <EngineCore/Assets/AssetImporter.hpp>
#include <Grindstone.Renderables.3D/include/Assets/RigAsset.hpp>

namespace Grindstone {
	class RigImporter : public SpecificAssetImporter<RigAsset, AssetType::Rig> {
	public:
		virtual ~RigImporter() override;

		virtual void* LoadAsset(Uuid uuid) override;
		virtual void QueueReloadAsset(Uuid uuid) override;
		virtual void OnDeleteAsset(Grindstone::RigAsset& asset) override;
	};
}
