#include <Grindstone.Renderables.3D/include/Assets/AnimationClipImporter.hpp>
#include <EngineCore/Assets/Loaders/AssetLoader.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Logger.hpp>
using namespace Grindstone;

static bool ImportAnimationClipFile(AnimationClipAsset& anim) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();

	Grindstone::Assets::AssetLoadBinaryResult result = engineCore.assetManager->LoadBinaryByUuid(AssetType::AnimationClip, anim.uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "AnimationClipImporter::LoadAsset Unable to load file with id: {}", anim.uuid.ToString());
		anim.assetLoadStatus = AssetLoadStatus::Missing;
		return false;
	}

	char* fileContent = reinterpret_cast<char*>(result.buffer.Get());
	uint64_t fileSize = result.buffer.GetCapacity();
	anim.name = result.displayName;

	return true;
}

AnimationClipImporter::~AnimationClipImporter() {}

void* AnimationClipImporter::LoadAsset(Uuid uuid) {
	auto animIterator = assets.emplace(uuid, AnimationClipAsset(uuid, uuid.ToString()));
	AnimationClipAsset& animAsset = animIterator.first->second;

	animAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!ImportAnimationClipFile(animAsset)) {
		return nullptr;
	}

	return &animAsset;
}

void AnimationClipImporter::QueueReloadAsset(Uuid uuid) {}
void AnimationClipImporter::OnDeleteAsset(Grindstone::AnimationClipAsset& asset) {}
