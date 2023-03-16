#include <stdexcept>
#include <filesystem>
#include <iostream>

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "EngineCore/Assets/AssetManager.hpp"
#include "EngineCore/Reflection/TypeDescriptorAsset.hpp"
#include "SceneLoaderJson.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/Math.hpp"
#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

SceneLoaderJson::SceneLoaderJson(Scene* scene, const char* path) : scene(scene), path(path) {
	Load(path);
}

bool SceneLoaderJson::Load(const char* path) {
	if (!std::filesystem::exists(path)) {
		std::string printMsg = std::string("Error loading scene: ") + path;
		EngineCore::GetInstance().Print(Grindstone::LogSeverity::Error, printMsg.c_str());
		return false;
	}

	size_t strLen = strlen(path) + 1;
	scene->path = (char*)malloc(strLen);
	strcpy_s(scene->path, strLen, path);
	std::string fileContents = Utils::LoadFileText(path);
	document.Parse(fileContents.c_str());

	std::string printMsg = std::string("Loading scene: ") + path;
	EngineCore::GetInstance().Print(Grindstone::LogSeverity::Info, printMsg.c_str());

	ProcessMeta();
	ProcessEntities();

	return true;
}

void SceneLoaderJson::ProcessMeta() {
	const char* name = document.HasMember("name")
		? document["name"].GetString()
		: "Untitled Scene";

	size_t strLen = strlen(path) + 1;
	scene->name = (char*)malloc(strLen);
	strcpy_s(scene->name, strLen, name);
}

void SceneLoaderJson::ProcessEntities() {
	for (auto& entity : document["entities"].GetArray()) {
		ProcessEntity(entity);
	}
}

void SceneLoaderJson::ProcessEntity(rapidjson::Value& entityJson) {
	auto entityId = entityJson["entityId"].GetUint();
	ECS::Entity entity = scene->CreateEntity((entt::entity)entityId);

	for (auto& component : entityJson["components"].GetArray()) {
		ProcessComponent(entity, component);
	}
}

void SceneLoaderJson::ProcessComponent(ECS::Entity entity, rapidjson::Value& component) {
	const char* componentType = component["component"].GetString();

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
				auto& parameter = parameterList.MemberBegin();
				parameter != parameterList.MemberEnd();
				parameter++
			) {
				const char* paramKey = parameter->name.GetString();
				ProcessComponentParameter(entity, componentPtr, reflectionData, paramKey, parameter->value);
			}
		}
	}

	componentRegistrar->SetupComponent(componentType, entity, componentPtr);
}

void CopyDataArrayFloat(rapidjson::Value& srcParameter, float* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetFloat();
	}
}

void CopyDataArrayDouble(rapidjson::Value& srcParameter, double* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetDouble();
	}
}

void CopyDataArrayInt(rapidjson::Value& srcParameter, int* dstArray, rapidjson::SizeType count) {
	auto srcArray = srcParameter.GetArray();

	for (rapidjson::SizeType i = 0; i < count; ++i) {
		dstArray[i] = srcArray[i].GetInt();
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

void SceneLoaderJson::ParseMember(
	void* memberPtr,
	Reflection::TypeDescriptor* member,
	rapidjson::Value& parameter
) {
	switch (member->type) {
	default:
		EngineCore::GetInstance().Print(LogSeverity::Warning, "Unhandled reflection type in SceneLoaderJson!");
		break;
	case ReflectionTypeData::AssetReference: {
		auto type = (Grindstone::Reflection::TypeDescriptor_AssetReference*)member;
		AssetType assetType = type->assetType;
		GenericAssetReference& assetReference = *(GenericAssetReference*)memberPtr;
		std::string uuidAsString = parameter.GetString();
		Uuid uuid = Uuid(uuidAsString);
		void* asset = EngineCore::GetInstance().assetManager->GetAsset(assetType, uuid);
		assetReference = GenericAssetReference{ uuid, asset };
		break;
	}
	case ReflectionTypeData::Struct:
		// TODO: Implement this
		EngineCore::GetInstance().Print(LogSeverity::Warning, "Unhandled Struct in SceneLoaderJson!");
		break;
	case ReflectionTypeData::Vector: {
		ParseArray(memberPtr, member, parameter);
		break;
	}
	case ReflectionTypeData::String: {
		std::string& str = *(std::string*)memberPtr;
		str = parameter.GetString();
		break;
	}
	case Reflection::TypeDescriptor::ReflectionTypeData::Quaternion:
		CopyDataArrayFloat(parameter, (float*)memberPtr, 4);
		break;
	case ReflectionTypeData::Bool: {
		bool& str = *(bool*)memberPtr;
		str = parameter.GetBool();
		break;
	}
	case ReflectionTypeData::Int: {
		int& str = *(int*)memberPtr;
		str = parameter.GetInt();
		break;
	}
	case ReflectionTypeData::Int2:
		CopyDataArrayInt(parameter, (int*)memberPtr, 2);
		break;
	case ReflectionTypeData::Int3:
		CopyDataArrayInt(parameter, (int*)memberPtr, 3);
		break;
	case ReflectionTypeData::Int4:
		CopyDataArrayInt(parameter, (int*)memberPtr, 4);
		break;
	case ReflectionTypeData::Float: {
		float& str = *(float*)memberPtr;
		str = parameter.GetFloat();
		break;
	}
	case ReflectionTypeData::Float2:
		CopyDataArrayFloat(parameter, (float*)memberPtr, 2);
		break;
	case ReflectionTypeData::Float3:
		CopyDataArrayFloat(parameter, (float*)memberPtr, 3);
		break;
	case ReflectionTypeData::Float4:
		CopyDataArrayFloat(parameter, (float*)memberPtr, 4);
		break;
	case ReflectionTypeData::Double: {
		double& str = *(double*)memberPtr;
		str = parameter.GetDouble();
		break;
	}
	case ReflectionTypeData::Double2:
		CopyDataArrayDouble(parameter, (double*)memberPtr, 2);
		break;
	case ReflectionTypeData::Double3:
		CopyDataArrayDouble(parameter, (double*)memberPtr, 3);
		break;
	case ReflectionTypeData::Double4:
		CopyDataArrayDouble(parameter, (double*)memberPtr, 4);
		break;
	}
}

template<typename T>
inline void SetupArray(void* memberPtr, size_t arraySize, void*& elementPtr, size_t& elementSize) {
	std::vector<T>& vector = *(std::vector<T>*)memberPtr;
	vector.resize(arraySize);
	elementSize = sizeof(T);
	elementPtr = (void*)&vector[0];
}

void SceneLoaderJson::ParseArray(void* memberPtr, Reflection::TypeDescriptor* member, rapidjson::Value& parameter) {
	Reflection::TypeDescriptor_StdVector* vectorTypeDescriptor = static_cast<Reflection::TypeDescriptor_StdVector*>(member);
	auto srcArray = parameter.GetArray();
	size_t elementSize = 0;
	size_t arraySize = static_cast<size_t>(srcArray.Size());
	void* elementPtr = nullptr;

	switch (vectorTypeDescriptor->itemType->type) {
		case ReflectionTypeData::Struct: break;
		case ReflectionTypeData::AssetReference:
			SetupArray<GenericAssetReference>(memberPtr, arraySize, elementPtr, elementSize);
			break;
		case ReflectionTypeData::Bool:
			SetupArray<bool>(memberPtr, arraySize, elementPtr, elementSize);
			break;
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
		case ReflectionTypeData::Int:
			SetupArray<Math::Int>(memberPtr, arraySize, elementPtr, elementSize);
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
	}

	std::vector<GenericAssetReference>* ar = (std::vector<GenericAssetReference>*)memberPtr;

	for (
		rapidjson::Value* elementIterator = srcArray.Begin();
		elementIterator != srcArray.End();
		++elementIterator
	) {
		ParseMember(
			elementPtr,
			vectorTypeDescriptor->itemType,
			*elementIterator
		);

		elementPtr = (char*)elementPtr + elementSize;
	}

	std::cout << ar->size() << std::endl;
}
