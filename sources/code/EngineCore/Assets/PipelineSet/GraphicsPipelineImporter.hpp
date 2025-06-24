#pragma once

#include <string>
#include <map>
#include <fstream>

#include "EngineCore/Assets/AssetImporter.hpp"
#include "GraphicsPipelineAsset.hpp"

namespace Grindstone {
	class BaseAssetRenderer;
	class GraphicsPipelineImporter : public SpecificAssetImporter<GraphicsPipelineAsset, AssetType::GraphicsPipelineSet> {
		public:
			virtual ~GraphicsPipelineImporter() override;

			virtual void* LoadAsset(Uuid uuid) override;
			virtual void QueueReloadAsset(Uuid uuid) override;

	};
}
