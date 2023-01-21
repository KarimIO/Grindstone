#include <stdexcept>
#include <filesystem>
#include <iostream>

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "SceneLoaderJson.hpp"
#include "EngineCore/Utils/Utilities.hpp"
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

	switch (member->type->type) {
		default:
			EngineCore::GetInstance().Print(LogSeverity::Warning, "Unhandled reflection type in SceneLoaderJson!");
			break;
		case ReflectionTypeData::Struct: {
			// TODO: Implement this
			EngineCore::GetInstance().Print(LogSeverity::Warning, "Unhandled Struct in SceneLoaderJson!");
			break;
		}
		case ReflectionTypeData::Vector: {
			auto srcArray = parameter.GetArray();
			std::vector<std::string>& vector = *(std::vector<std::string>*)memberPtr;
			vector.reserve(srcArray.Size());

			for (
				rapidjson::Value* elementIterator = srcArray.Begin();
				elementIterator != srcArray.End();
				++elementIterator
			) {
				vector.push_back(elementIterator->GetString());
			}
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
