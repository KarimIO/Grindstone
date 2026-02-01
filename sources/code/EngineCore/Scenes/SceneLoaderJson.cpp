#include <stdexcept>
#include <filesystem>
#include <iostream>

#include <Common/PhysicsLayer.hpp>
#include "Common/Math.hpp"
#include "EngineCore/Profiling.hpp"
#include "EngineCore/EngineCore.hpp"
#include <EngineCore/Logger.hpp>
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/Reflection/TypeDescriptorAsset.hpp"
#include "EngineCore/WorldContext/WorldContextManager.hpp"
#include "EngineCore/Utils/Utilities.hpp"

#include "SceneLoaderJson.hpp"
#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;


static void ParseMember( void* memberPtr, Reflection::TypeDescriptor* member, rapidjson::Value& parameter);
static void ParseFixedArray(void* memberPtr, Reflection::TypeDescriptor* member, rapidjson::Value& parameter);
static void ParseVectorArray(void* memberPtr, Reflection::TypeDescriptor* member, rapidjson::Value& parameter);

SceneLoaderJson::SceneLoaderJson(Scene* scene, Grindstone::Uuid uuid) : scene(scene), uuid(uuid) {
	Load(uuid);
}

bool SceneLoaderJson::Load(Grindstone::Uuid uuid) {
	EngineCore& engineCore = EngineCore::GetInstance();
	Assets::AssetManager* assetManager = engineCore.assetManager;

	Assets::AssetLoadTextResult result = engineCore.assetManager->LoadTextByUuid(AssetType::Scene, uuid);
	if (result.status != Assets::AssetLoadStatus::Success) {
		GPRINT_ERROR_V(LogSource::EngineCore, "Could not find scene with id {}.", uuid.ToString());
		return false;
	}

	scene->path = result.displayName;

	rapidjson::ParseResult parseResult = document.Parse(result.content.c_str());

	if (parseResult.IsError()) {
		rapidjson::GetParseErrorFunc GetParseError = rapidjson::GetParseErrorFunc();
		const RAPIDJSON_ERROR_CHARTYPE* errorCode = "Unknown Error";
		if (GetParseError != nullptr) {
			errorCode = GetParseError(parseResult.Code());
		}
		GPRINT_ERROR_V(LogSource::EngineCore, "Failed to load scene '{}' with id {} - Got error '{}' with offset {}.", result.displayName, uuid.ToString(), errorCode, document.GetErrorOffset());
		return false;
	}

	GPRINT_INFO_V(LogSource::EngineCore, "Loading scene '{}' with id {}.", result.displayName, uuid.ToString());

	ProcessMeta();
	ProcessEntities();

	return true;
}

void SceneLoaderJson::ProcessMeta() {
	const char* name = document.HasMember("name")
		? document["name"].GetString()
		: "Untitled Scene";

	scene->name = name;
}

void SceneLoaderJson::ProcessEntities() {
	for (auto& entity : document["entities"].GetArray()) {
		ProcessEntity(entity);
	}

	EngineCore& engineCore = EngineCore::GetInstance();
	auto componentRegistrar = engineCore.GetComponentRegistrar();
	componentRegistrar->CallCreateOnRegistry(*engineCore.GetWorldContextManager()->GetActiveWorldContextSet());
}

void SceneLoaderJson::ProcessEntity(rapidjson::Value& entityJson) {
	auto entityId = entityJson["entityId"].GetUint();

	{
#if _DEBUG
		std::string scopeName = "Loading entity with id: " + std::to_string(entityId);
		GRIND_PROFILE_SCOPE(scopeName.c_str());
#endif
		ECS::Entity entity = scene->CreateEntity(static_cast<entt::entity>(entityId));

		for (auto& component : entityJson["components"].GetArray()) {
			ProcessComponent(entity, component);
		}
	}
}

void SceneLoaderJson::ProcessComponent(ECS::Entity entity, rapidjson::Value& component) {
	Grindstone::HashedString componentType = Grindstone::HashedString(component["component"].GetString());

	Reflection::TypeDescriptor_Struct reflectionData;
	auto componentRegistrar = EngineCore::GetInstance().GetComponentRegistrar();
	if (!componentRegistrar->TryGetComponentReflectionData(componentType, reflectionData)) {
		return;
	}

	void* componentPtr = nullptr;
	if (entity.HasComponent(componentType)) {
		componentPtr = entity.GetComponent(componentType);
	}
	else {
		componentPtr = entity.AddComponentWithoutSetup(componentType);
	}

	if (component.HasMember("params")) {
		auto& parameterList = component["params"];

		if (parameterList.IsObject()) {
			for (
				auto parameter = parameterList.MemberBegin();
				parameter != parameterList.MemberEnd();
				parameter++
			) {
				const char* paramKey = parameter->name.GetString();
				ProcessComponentParameter(entity, componentPtr, reflectionData, paramKey, parameter->value);
			}
		}
	}
}

static void CopyDataArrayFloat(rapidjson::Value& srcParameter, float* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetFloat();
	}
}

static void CopyDataArrayDouble(rapidjson::Value& srcParameter, double* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetDouble();
	}
}

static void CopyDataArrayInt(rapidjson::Value& srcParameter, int* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetInt();
	}
}

static void CopyDataArrayUint(rapidjson::Value& srcParameter, uint32_t* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetUint();
	}
}

using ReflectionTypeData = Reflection::TypeDescriptor_Struct::ReflectionTypeData;
void SceneLoaderJson::ProcessComponentParameter(
	ECS::Entity entity,
	void* componentPtr,
	Reflection::TypeDescriptor_Struct& reflectionData,
	const char* parameterKey,
	rapidjson::Value& parameter
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
	char* memberPtr = (char*)componentPtr + offset;
	ParseMember(memberPtr, member->type, parameter);
}

static void ParseMember(
	void* memberPtr,
	Reflection::TypeDescriptor* member,
	rapidjson::Value& parameter
) {
	switch (member->type) {
	default:
		GPRINT_ERROR_V(LogSource::EngineCore, "Unhandled reflection type '{}' in SceneLoaderJson!", member->GetFullName());
		break;
	case ReflectionTypeData::Entity: {
		entt::entity& entity = *(entt::entity*)memberPtr;
		entity = static_cast<entt::entity>(parameter.GetUint());
		break;
	}
	case ReflectionTypeData::PhysicsLayer: {
		Grindstone::Physics::Layer& layer = *(Grindstone::Physics::Layer*)memberPtr;
		layer.layer = static_cast<uint8_t>(parameter.GetUint());
		break;
	}
	case ReflectionTypeData::PhysicsLayerMask: {
		Grindstone::Physics::LayerMask& mask = *(Grindstone::Physics::LayerMask*)memberPtr;
		mask.mask = static_cast<uint32_t>(parameter.GetUint());
		break;
	}
	case ReflectionTypeData::AssetReference: {
		GenericAssetReference& assetRefPtr = *static_cast<GenericAssetReference*>(memberPtr);
		auto type = (Grindstone::Reflection::TypeDescriptor_AssetReference*)member;
		const char* uuidAsString = parameter.GetString();
		if (Grindstone::Uuid::MakeFromString(uuidAsString, assetRefPtr.uuid)) {
			EngineCore::GetInstance().assetManager->GetAndIncrementAssetCount(type->assetType, assetRefPtr.uuid);
		}
		else {
			GPRINT_ERROR(LogSource::EngineCore, "Invalid UUID in entity!");
		}
		break;
	}
	case ReflectionTypeData::Struct:
		// TODO: Implement this
		GPRINT_ERROR(LogSource::EngineCore, "Unhandled Struct in SceneLoaderJson!");
		break;
	case ReflectionTypeData::Vector: {
		ParseVectorArray(memberPtr, member, parameter);
		break;
	}
	case ReflectionTypeData::FixedArray: {
		ParseFixedArray(memberPtr, member, parameter);
		break;
	}
	case ReflectionTypeData::String: {
		std::string& str = *static_cast<std::string*>(memberPtr);
		str = parameter.GetString();
		break;
	}
	case Reflection::TypeDescriptor::ReflectionTypeData::Quaternion:
		CopyDataArrayFloat(parameter, static_cast<float*>(memberPtr), 4);
		break;
	case ReflectionTypeData::Bool: {
		bool& str = *(bool*)memberPtr;
		str = parameter.GetBool();
		break;
	}
	case ReflectionTypeData::Int8: {
		int8_t& str = *(int8_t*)memberPtr;
		str = parameter.GetInt();
		break;
	}
	case ReflectionTypeData::Int16: {
		int16_t& str = *(int16_t*)memberPtr;
		str = parameter.GetInt();
		break;
	}
	case ReflectionTypeData::Int32: {
		int32_t& str = *(int32_t*)memberPtr;
		str = parameter.GetInt();
		break;
	}
	case ReflectionTypeData::Int64: {
		int64_t& str = *(int64_t*)memberPtr;
		str = parameter.GetInt64();
		break;
	}
	case ReflectionTypeData::Int2:
		CopyDataArrayInt(parameter, static_cast<int*>(memberPtr), 2);
		break;
	case ReflectionTypeData::Int3:
		CopyDataArrayInt(parameter, static_cast<int*>(memberPtr), 3);
		break;
	case ReflectionTypeData::Int4:
		CopyDataArrayInt(parameter, static_cast<int*>(memberPtr), 4);
		break;
	case ReflectionTypeData::Uint8: {
		uint8_t& str = *(uint8_t*)memberPtr;
		str = parameter.GetUint();
		break;
	}
	case ReflectionTypeData::Uint16: {
		uint16_t& str = *(uint16_t*)memberPtr;
		str = parameter.GetUint();
		break;
	}
	case ReflectionTypeData::Uint32: {
		uint32_t& str = *(uint32_t*)memberPtr;
		str = parameter.GetUint();
		break;
	}
	case ReflectionTypeData::Uint64: {
		uint64_t& str = *(uint64_t*)memberPtr;
		str = parameter.GetUint64();
		break;
	}
	case ReflectionTypeData::Uint2:
		CopyDataArrayUint(parameter, static_cast<uint32_t*>(memberPtr), 2);
		break;
	case ReflectionTypeData::Uint3:
		CopyDataArrayUint(parameter, static_cast<uint32_t*>(memberPtr), 3);
		break;
	case ReflectionTypeData::Uint4:
		CopyDataArrayUint(parameter, static_cast<uint32_t*>(memberPtr), 4);
		break;
	case ReflectionTypeData::Float: {
		float& str = *(float*)memberPtr;
		str = parameter.GetFloat();
		break;
	}
	case ReflectionTypeData::Float2:
		CopyDataArrayFloat(parameter, static_cast<float*>(memberPtr), 2);
		break;
	case ReflectionTypeData::Float3:
		CopyDataArrayFloat(parameter, static_cast<float*>(memberPtr), 3);
		break;
	case ReflectionTypeData::Float4:
		CopyDataArrayFloat(parameter, static_cast<float*>(memberPtr), 4);
		break;
	case ReflectionTypeData::Double: {
		double& str = *static_cast<double*>(memberPtr);
		str = parameter.GetDouble();
		break;
	}
	case ReflectionTypeData::Double2:
		CopyDataArrayDouble(parameter, static_cast<double*>(memberPtr), 2);
		break;
	case ReflectionTypeData::Double3:
		CopyDataArrayDouble(parameter, static_cast<double*>(memberPtr), 3);
		break;
	case ReflectionTypeData::Double4:
		CopyDataArrayDouble(parameter, static_cast<double*>(memberPtr), 4);
		break;
	}
}

template<typename T>
inline void SetupArray(void* memberPtr, size_t arraySize, void*& elementPtr, size_t& elementSize) {
	std::vector<T>& vector = *(std::vector<T>*)memberPtr;
	vector.resize(arraySize);
	elementSize = sizeof(T);
	T* vectorPtr = vector.data();
	elementPtr = reinterpret_cast<void*>(vectorPtr);
}

static void ParseArray(void* memberPtr, Reflection::TypeDescriptor* itemType, rapidjson::Value& parameter) {
	auto srcArray = parameter.GetArray();

	if (srcArray.Size() == 0) {
		return;
	}

	size_t elementSize = 0;
	size_t arraySize = static_cast<size_t>(srcArray.Size());
	void* elementPtr = nullptr;

	switch (itemType->type) {
		case ReflectionTypeData::Struct: break;
		case ReflectionTypeData::AssetReference:
			SetupArray<GenericAssetReference>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		// Special case for bool because bool vectors are bit arrays
		case ReflectionTypeData::Bool: {
			std::vector<bool>& vector = *(std::vector<bool>*)memberPtr;
			vector.clear();
			vector.reserve(arraySize);
			for (
				rapidjson::Value* elementIterator = srcArray.Begin();
				elementIterator != srcArray.End();
				++elementIterator
			) {
				vector.push_back(parameter.GetBool());
			}
			return;
		}
		case ReflectionTypeData::Quaternion:
			SetupArray<Math::Quaternion>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::String:
			SetupArray<std::string>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Vector:
			SetupArray<std::vector<char>>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Double:
			SetupArray<Math::Double>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Double2:
			SetupArray<Math::Double2>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Double3:
			SetupArray<Math::Double3>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Double4:
			SetupArray<Math::Double4>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Int8:
			SetupArray<int8_t>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Int16:
			SetupArray<int16_t>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Int32:
			SetupArray<int32_t>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Int64:
			SetupArray<int64_t>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Int2:
			SetupArray<Math::Int2>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Int3:
			SetupArray<Math::Int3>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Int4:
			SetupArray<Math::Int4>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Uint8:
			SetupArray<uint8_t>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Uint16:
			SetupArray<uint16_t>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Uint32:
			SetupArray<uint32_t>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Uint64:
			SetupArray<uint64_t>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Uint2:
			SetupArray<Math::Uint2>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Uint3:
			SetupArray<Math::Uint3>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Uint4:
			SetupArray<Math::Uint4>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Float:
			SetupArray<Math::Float>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Float2:
			SetupArray<Math::Float2>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Float3:
			SetupArray<Math::Float3>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Float4:
			SetupArray<Math::Float4>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Entity:
			SetupArray<entt::entity>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::PhysicsLayer:
			SetupArray<Grindstone::Physics::Layer>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::PhysicsLayerMask:
			SetupArray<Grindstone::Physics::LayerMask>(memberPtr, arraySize, elementPtr, elementSize);
			break;
	}

	for (
		rapidjson::Value* elementIterator = srcArray.Begin();
		elementIterator != srcArray.End();
		++elementIterator
	) {
		ParseMember(
			elementPtr,
			itemType,
			*elementIterator
		);

		elementPtr = (char*)elementPtr + elementSize;
	}
}

static void ParseFixedArray(void* memberPtr, Reflection::TypeDescriptor* member, rapidjson::Value& parameter) {
	Reflection::TypeDescriptor_FixedArray* vectorTypeDescriptor = static_cast<Reflection::TypeDescriptor_FixedArray*>(member);
	ParseArray(memberPtr, vectorTypeDescriptor->itemType, parameter);
}

static void ParseVectorArray(void* memberPtr, Reflection::TypeDescriptor* member, rapidjson::Value& parameter) {
	Reflection::TypeDescriptor_StdVector* vectorTypeDescriptor = static_cast<Reflection::TypeDescriptor_StdVector*>(member);
	ParseArray(memberPtr, vectorTypeDescriptor->itemType, parameter);
}
