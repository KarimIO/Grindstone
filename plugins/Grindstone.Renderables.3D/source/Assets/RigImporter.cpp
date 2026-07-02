#include <EngineCore/Assets/Loaders/AssetLoader.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Logger.hpp>

#include <Grindstone.Renderables.3D/include/Assets/RigImporter.hpp>

using namespace Grindstone;

static bool ImportRigFile(RigAsset& anim) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();

	Grindstone::Assets::AssetLoadBinaryResult result = engineCore.assetManager->LoadBinaryByUuid(AssetType::Rig, anim.uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "RigImporter::LoadAsset Unable to load file with id: {}", anim.uuid.ToString());
		anim.assetLoadStatus = AssetLoadStatus::Missing;
		return false;
	}

	char* fileContent = reinterpret_cast<char*>(result.buffer.Get());
	uint64_t fileSize = result.buffer.GetCapacity();
	anim.name = result.displayName;

	return true;
}

RigImporter::~RigImporter() {}

void* RigImporter::LoadAsset(Uuid uuid) {
	auto rigIterator = assets.emplace(uuid, RigAsset(uuid, uuid.ToString()));
	RigAsset& rigAsset = rigIterator.first->second;

	rigAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!ImportRigFile(rigAsset)) {
		return nullptr;
	}

	return &rigAsset;
}

void RigImporter::QueueReloadAsset(Uuid uuid) {}
void RigImporter::OnDeleteAsset(Grindstone::RigAsset& asset) {}
