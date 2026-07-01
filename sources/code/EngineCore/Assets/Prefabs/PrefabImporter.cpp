#include <rapidjson/document.h>

#include <EngineCore/Assets/Loaders/AssetLoader.hpp>
#include <EngineCore/Assets/AssetManager.hpp>
#include <EngineCore/Profiling.hpp>
#include <EngineCore/Logger.hpp>
#include <Common/PhysicsLayer.hpp>

#include "PrefabImporter.hpp"

using namespace Grindstone;

static void ProcessEntities(const rapidjson::Document& document, PrefabAsset& prefabAsset) {
	for (const rapidjson::Value& entity : document["entities"].GetArray()) {
		ProcessEntity(entity, prefabAsset);
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	auto componentRegistrar = engineCore.GetComponentRegistrar();
	componentRegistrar->CallCreateOnRegistry(*engineCore.GetWorldContextManager()->GetActiveWorldContextSet());
}

static void ProcessEntity(const rapidjson::Value& entityJson, PrefabAsset& prefabAsset) {
	auto entityId = entityJson["entityId"].GetUint();

	PrefabAsset::PrefabEntityDescription& entityDesc = prefabAsset.entitiesDescriptions.emplace_back();

	for (auto& component : entityJson["components"].GetArray()) {
		ProcessComponent(component, entityDesc);
	}
}

static void ProcessComponent(const rapidjson::Value& component, PrefabAsset::PrefabEntityDescription& entityDesc) {
	Grindstone::HashedString componentType = Grindstone::HashedString(component["component"].GetString());

	Reflection::TypeDescriptor_Struct reflectionData;
	auto componentRegistrar = EngineCore::GetInstance().GetComponentRegistrar();
	if (!componentRegistrar->TryGetComponentReflectionData(componentType, reflectionData)) {
		return;
	}

	PrefabAsset::ComponentDescriptor& compDesc = entityDesc.components.emplace_back();

	if (component.HasMember("params")) {
		const rapidjson::Value& parameterList = component["params"];

		if (parameterList.IsObject()) {
			for (
				auto parameter = parameterList.MemberBegin();
				parameter != parameterList.MemberEnd();
				parameter++
			) {
				const char* paramKey = parameter->name.GetString();
				ProcessComponentParameter(reflectionData, paramKey, parameter->value, compDesc);
			}
		}
	}
}

static void CopyDataArrayFloat(const rapidjson::Value& srcParameter, float* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetFloat();
	}
}

static void CopyDataArrayDouble(const rapidjson::Value& srcParameter, double* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetDouble();
	}
}

static void CopyDataArrayInt(const rapidjson::Value& srcParameter, int* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetInt();
	}
}

static void CopyDataArrayUint(const rapidjson::Value& srcParameter, uint32_t* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetUint();
	}
}

using ReflectionTypeData = Reflection::TypeDescriptor_Struct::ReflectionTypeData;
static void ProcessComponentParameter(
	Reflection::TypeDescriptor_Struct& reflectionData,
	const char* parameterKey,
	const rapidjson::Value& parameter,
	PrefabAsset::ComponentDescriptor& compDesc
) {
	Reflection::TypeDescriptor_Struct::Member* member = nullptr;
	for (auto& itrMember : reflectionData.category.members) {
		if (itrMember.storedName == parameterKey) {
			member = &itrMember;
			break;
		}
	}

	if (member == nullptr) {
		return;
	}

	auto offset = member->offset;
	PrefabAsset::ComponentDataDescriptor& data = compDesc.fields.emplace_back();
	ParseMember(memberPtr, member->type, parameter);
}

static bool LoadPrefabAsset(PrefabAsset& prefabAsset) {
	EngineCore& engineCore = EngineCore::GetInstance();

	Assets::AssetLoadTextResult result = engineCore.assetManager->LoadTextByUuid(AssetType::Prefab, prefabAsset.uuid);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not find scene with id {}.", prefabAsset.uuid.ToString());
		return false;
	}

	rapidjson::Document document;
	rapidjson::ParseResult parseResult = document.Parse(result.content.c_str());

	if (parseResult.IsError()) {
		rapidjson::GetParseErrorFunc GetParseError = rapidjson::GetParseErrorFunc();
		const RAPIDJSON_ERROR_CHARTYPE* errorCode = "Unknown Error";
		if (GetParseError != nullptr) {
			errorCode = GetParseError(parseResult.Code());
		}
		GPRINT_ERROR_V(LogSource::EngineCore, "Failed to load prefab '{}' with id {} - Got error '{}' with offset {}.", result.displayName, prefabAsset.uuid.ToString(), errorCode, document.GetErrorOffset());
		return false;
	}

	GPRINT_INFO_V(LogSource::EngineCore, "Loading prefab '{}' with id {}.", result.displayName, prefabAsset.uuid.ToString());

	ProcessEntities(document, prefabAsset);
}

Grindstone::PrefabImporter::~PrefabImporter() {
}

void* Grindstone::PrefabImporter::LoadAsset(Uuid uuid) {
	auto textureIterator = assets.emplace(uuid, PrefabAsset(uuid));
	PrefabAsset& prefabAsset = textureIterator.first->second;

	prefabAsset.assetLoadStatus = AssetLoadStatus::Loading;
	if (!LoadPrefabAsset(prefabAsset)) {
		return nullptr;
	}

	return &prefabAsset;
}

void Grindstone::PrefabImporter::QueueReloadAsset(Uuid uuid) {
	auto prefabIterator = assets.find(uuid);
	if (prefabIterator == assets.end()) {
		return;
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	prefabIterator->second.assetLoadStatus = AssetLoadStatus::Reloading;

	LoadPrefabAsset(prefabIterator->second);
}
