#pragma once

#include <string>
#include <map>
#include <fstream>

#include "EngineCore/Assets/AssetImporter.hpp"
#include "ComputePipelineAsset.hpp"

namespace Grindstone {
	class BaseAssetRenderer;
	class ComputePipelineImporter : public SpecificAssetImporter<ComputePipelineAsset> {
		public:
			virtual ~ComputePipelineImporter() override;

			virtual void* LoadAsset(Uuid uuid) override;
			virtual void QueueReloadAsset(Uuid uuid) override;

	};
}
