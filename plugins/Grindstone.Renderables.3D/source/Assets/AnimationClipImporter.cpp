#include <Grindstone.Renderables.3D/include/Assets/AnimationClipImporter.hpp>
#include <EngineCore/Assets/Loaders/AssetLoader.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Logger.hpp>
#include <Common/Formats/Animation.hpp>

using namespace Grindstone;
using namespace Grindstone::Containers;
using namespace Grindstone::Formats::Animation;

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

	size_t sizeOfMagic = 4;

	if (result.buffer.GetCapacity() <= sizeOfMagic + sizeof(V1::Header)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "AnimationClipImporter::LoadAsset Failed to read file '{}' because it is too small (does not fit magic + header).", result.displayName.c_str());
		return false;
	}

	if (strncmp(reinterpret_cast<const char*>(result.buffer.Get()), "GAF", sizeOfMagic)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "AnimationClipImporter::LoadAsset Failed to read file '{}' because it starts with an invalid magic code (not GAF).", result.displayName.c_str());
		return false;
	}

	const V1::Header& animHeader = reinterpret_cast<V1::Header&>(*result.buffer.Get(sizeOfMagic));

	if (result.buffer.GetCapacity() != animHeader.totalFileSize) {
		GPRINT_ERROR_V(LogSource::EngineCore, "AnimationClipImporter::LoadAsset Failed to read file '{}' file size {} does not match totalFileSize {} in header.", result.displayName.c_str(), result.buffer.GetCapacity(), animHeader.totalFileSize);
		return false;
	}

	if (animHeader.version != 1) {
		GPRINT_ERROR_V(LogSource::EngineCore, "AnimationClipImporter::LoadAsset Failed to read file '{}' version in header {} does not match expected version {}.", result.displayName.c_str(), animHeader.version, 1);
		return false;
	}

	anim.ticksPerSecond = animHeader.ticksPerSecond;
	anim.duration = animHeader.animationDuration;

	anim.positions.resize(animHeader.positionKeyframesCount);
	anim.rotations.resize(animHeader.rotationKeyframesCount);
	anim.scales.resize(animHeader.scaleKeyframesCount);

	std::memcpy(anim.positions.data(), result.buffer.Get(animHeader.positionKeyframesOffset), animHeader.positionKeyframesCount * sizeof(AnimationClipAsset::PositionKeyframe));
	std::memcpy(anim.rotations.data(), result.buffer.Get(animHeader.rotationKeyframesOffset), animHeader.rotationKeyframesCount * sizeof(AnimationClipAsset::RotationKeyframe));
	std::memcpy(anim.scales.data(), result.buffer.Get(animHeader.scaleKeyframesOffset), animHeader.scaleKeyframesCount * sizeof(AnimationClipAsset::ScaleKeyframe));

	char* stringBuffer = reinterpret_cast<char*>(result.buffer.Get(animHeader.stringBlockOffset));

	const Span<V1::BoneChannel> srcChannels = result.buffer.GetSpan<V1::BoneChannel>(animHeader.boneChannelDataOffset, animHeader.boneChannelCount);
	anim.boneChannels.reserve(srcChannels.GetSize());

	for (size_t i = 0; i < srcChannels.GetSize(); ++i) {
		const V1::BoneChannel& srcChannel = srcChannels[i];
		AnimationClipAsset::BoneChannel& dstChannel = anim.boneChannels.emplace_back();

		dstChannel.boneName = stringBuffer + srcChannel.boneNameStringOffset;
		dstChannel.positionKeyOffset = srcChannel.positionKeyOffset;
		dstChannel.rotationKeyOffset = srcChannel.rotationKeyOffset;
		dstChannel.scaleKeyOffset = srcChannel.scaleKeyOffset;
		dstChannel.positionCount = srcChannel.positionCount;
		dstChannel.rotationCount = srcChannel.rotationCount;
		dstChannel.scaleCount = srcChannel.scaleCount;
		dstChannel.interpolation = static_cast<AnimationClipAsset::KeyframeInterpolation>(srcChannel.interpolation);
	}

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
