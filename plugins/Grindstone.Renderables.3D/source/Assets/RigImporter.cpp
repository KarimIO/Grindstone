#include <EngineCore/Assets/Loaders/AssetLoader.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Logger.hpp>
#include <Common/Formats/Rig.hpp>

#include <Grindstone.Renderables.3D/include/Assets/RigImporter.hpp>

using namespace Grindstone;
using namespace Grindstone::Containers;
using namespace Grindstone::Formats::Rig;

static bool ImportRigFile(RigAsset& rig) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();

	Grindstone::Assets::AssetLoadBinaryResult result = engineCore.assetManager->LoadBinaryByUuid(AssetType::Rig, rig.uuid);
	if (result.status != Grindstone::Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "RigImporter::LoadAsset Unable to load file with id: {}", rig.uuid.ToString());
		rig.assetLoadStatus = AssetLoadStatus::Missing;
		return false;
	}

	rig.assetLoadStatus = AssetLoadStatus::Loading;
	char* fileContent = reinterpret_cast<char*>(result.buffer.Get());
	uint64_t fileSize = result.buffer.GetCapacity();
	rig.name = result.displayName;

	const char* desiredMagicCode = V1::magicCode;
	const uint32_t desiredVersion = V1::version;
	const uint32_t sizeOfMagic = V1::magicSize;

	if (result.buffer.GetCapacity() <= sizeOfMagic + sizeof(V1::Header)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "RigImporter::LoadAsset Failed to read file '{}' because it is too small (does not fit magic + header).", result.displayName.c_str());
		rig.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	const char* actualMagic = reinterpret_cast<const char*>(result.buffer.Get());
	if (strncmp(actualMagic, desiredMagicCode, sizeOfMagic)) {
		GPRINT_ERROR_V(LogSource::EngineCore, "RigImporter::LoadAsset Failed to read file '{}' because it starts with an invalid magic code '{}' (not '{}').", result.displayName.c_str(), actualMagic, desiredMagicCode);
		rig.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	const V1::Header& rigHeader = reinterpret_cast<V1::Header&>(*result.buffer.Get(sizeOfMagic));

	if (result.buffer.GetCapacity() != rigHeader.totalFileSize) {
		GPRINT_ERROR_V(LogSource::EngineCore, "RigImporter::LoadAsset Failed to read file '{}' file size {} does not match totalFileSize {} in header.", result.displayName.c_str(), result.buffer.GetCapacity(), rigHeader.totalFileSize);
		rig.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	if (rigHeader.version != desiredVersion) {
		GPRINT_ERROR_V(LogSource::EngineCore, "RigImporter::LoadAsset Failed to read file '{}' version in header {} does not match expected version {}.", result.displayName.c_str(), rigHeader.version, desiredVersion);
		rig.assetLoadStatus = AssetLoadStatus::Failed;
		return false;
	}

	char* stringBuffer = reinterpret_cast<char*>(result.buffer.Get(rigHeader.stringBlockOffset));

	const Span<V1::Bone> srcBones = result.buffer.GetSpan<V1::Bone>(rigHeader.boneDataOffset, rigHeader.bonesCount);
	rig.bones.reserve(rigHeader.bonesCount);

	auto& boneMap = rig.boneNameToIndex;
	
	for (size_t i = 0; i < srcBones.GetSize(); ++i) {
		const V1::Bone& srcBone = srcBones[i];
		RigAsset::Bone& dstBone = rig.bones.emplace_back();

		const char* boneName = stringBuffer + srcBone.boneNameStringOffset;
		GS_ASSERT(boneMap.find(boneName) == boneMap.end());
		boneMap[boneName] = i;

		dstBone.parentBoneIndex = srcBone.boneParentIndex;
		dstBone.localBindTransform = srcBone.localBindTransform;
		dstBone.inverseBindTransform = srcBone.inverseBindTransform;
	}

	rig.globalInverseTransform = rigHeader.globalInverseTransform;
	rig.assetLoadStatus = AssetLoadStatus::Ready;
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
