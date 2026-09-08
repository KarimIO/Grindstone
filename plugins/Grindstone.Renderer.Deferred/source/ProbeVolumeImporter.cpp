#include <filesystem>

#include <Common/Containers/Span.hpp>
#include <Common/Graphics/Core.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/Utils/Utilities.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/AssetRenderer/AssetRendererManager.hpp>
#include <EngineCore/Rendering/RenderPassRegistry.hpp>

#include <Grindstone.Renderer.Deferred/include/ProbeVolumeImporter.hpp>

using namespace Grindstone;
using namespace Grindstone::Containers;
using namespace Grindstone::GraphicsAPI;

static bool ImportProbeVolumeAsset(ProbeVolumeAsset& probeVolumeAsset) {
	EngineCore& engineCore = EngineCore::GetInstance();
	GraphicsAPI::Core* graphicsCore = engineCore.GetGraphicsCore();
	Assets::AssetManager* assetManager = engineCore.assetManager;
	AssetRendererManager* assetRendererManager = engineCore.assetRendererManager;
	RenderPassRegistry* renderPassRegistry = Grindstone::EngineCore::GetInstance().GetRenderPassRegistry();

	Assets::AssetLoadBinaryResult result = assetManager->LoadBinaryByUuid(AssetType::ProbeVolume, probeVolumeAsset.uuid);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR(LogSource::EngineCore, "Could not find ProbeVolume with id {}.", probeVolumeAsset.uuid.ToString());
		probeVolumeAsset.assetLoadStatus = AssetLoadStatus::Missing;
		return false;
	}

	/*
	Grindstone::Buffer& fileData = result.buffer;
	GS_ASSERT(fileData.GetCapacity() >= (4 + sizeof(V1::ProbeVolumeFileHeader)));

	if (memcmp(fileData.Get(), V1::FileMagicCode, 4) != 0) {
		GPRINT_ERROR(LogSource::EngineCore, "ProbeVolume file does not start with GPVD - {}.", result.displayName);
		ProbeVolumeAsset.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}
	*/

	probeVolumeAsset.assetLoadStatus = AssetLoadStatus::Ready;
	return true;
}

void* ProbeVolumeImporter::LoadAsset(Uuid uuid) {
	auto pipelineIterator = assets.emplace(uuid, ProbeVolumeAsset(uuid));
	Grindstone::ProbeVolumeAsset& ProbeVolumeAsset = pipelineIterator.first->second;

	ProbeVolumeAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!ImportProbeVolumeAsset(ProbeVolumeAsset)) {
		return nullptr;
	}

	return &ProbeVolumeAsset;
}

void ProbeVolumeImporter::QueueReloadAsset(Uuid uuid) {
	auto pipelineIterator = assets.find(uuid);
	if (pipelineIterator == assets.end()) {
		return;
	}

	Grindstone::ProbeVolumeAsset& ProbeVolumeAsset = pipelineIterator->second;

	ProbeVolumeAsset.assetLoadStatus = AssetLoadStatus::Reloading;

	ImportProbeVolumeAsset(ProbeVolumeAsset);
}

ProbeVolumeImporter::~ProbeVolumeImporter() {
	assets.clear();
}
