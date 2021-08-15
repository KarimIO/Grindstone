#include <stdexcept>
#include <filesystem>
#include <iostream>

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "SceneLoaderJson.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Scene.hpp"

#include "EngineCore/CoreComponents/Mesh/MeshComponent.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dManager.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

SceneLoaderJson::SceneLoaderJson(Scene* scene, const char* path) : scene(scene), path(path) {
	Load(path);
}

bool SceneLoaderJson::Load(const char* path) {
	if (!std::filesystem::exists(path)) {
		return false;
	}

	scene->path = path;
	std::string fileContents = Utils::LoadFileText(path);
	document.Parse(fileContents.c_str());

	ProcessMeta();
	ProcessEntities();

	auto mesh3dManager = EngineCore::GetInstance().mesh3dManager;
	auto& meshView = scene->GetEntityRegistry().view<const MeshComponent>();
	meshView.each([&](const MeshComponent& meshComponent) {
		mesh3dManager->LoadMesh3d(meshComponent.meshPath.c_str());
	});

	return true;
}

void SceneLoaderJson::ProcessMeta() {
	scene->name = document.HasMember("name")
		? document["name"].GetString()
		: "Untitled Scene";
}

void SceneLoaderJson::ProcessEntities() {
	auto& entities = document["entities"].GetArray();
	for (
		rapidjson::Value* entityIterator = entities.Begin();
		entityIterator != entities.End();
		++entityIterator
	) {
		auto& entity = entityIterator->GetObject();
		ProcessEntity(entity);
	}
}

void SceneLoaderJson::ProcessEntity(rapidjson::GenericObject<false, rapidjson::Value>& entityJson) {
	ECS::Entity entity = scene->CreateEntity();

	auto& components = entityJson["components"].GetArray();
	for (
		rapidjson::Value* componentIterator = components.Begin();
		componentIterator != components.End();
		++componentIterator
	) {
		auto& component = componentIterator->GetObject();
		ProcessComponent(entity, component);
	}
}

void SceneLoaderJson::ProcessComponent(ECS::Entity entity, rapidjson::GenericObject<false, rapidjson::Value>& component) {
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
		componentPtr = entity.AddComponent(componentType);
	}

	if (!component.HasMember("params")) {
		return;
	}

	auto& parameters = component["params"].GetObject();
	for (
		auto parameterIterator = parameters.MemberBegin();
		parameterIterator != parameters.MemberEnd();
		++parameterIterator
	) {
		const char* paramKey = parameterIterator->name.GetString();
		ProcessComponentParameter(entity, componentPtr, reflectionData, paramKey, parameterIterator->value);
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

	switch(member->type->type) {
		case ReflectionTypeData::Vector:
		case ReflectionTypeData::String: {
			std::string& str = *(std::string*)memberPtr;
			str = parameter.GetString();
			break;
		}
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
		case ReflectionTypeData::Int3:
		case ReflectionTypeData::Int4:
		case ReflectionTypeData::Float: {
			float& str = *(float*)memberPtr;
			str = parameter.GetFloat();
			break;
		}
		case ReflectionTypeData::Float2:
		case ReflectionTypeData::Float3:
		case ReflectionTypeData::Float4:
		case ReflectionTypeData::Double: {
			double& str = *(double*)memberPtr;
			str = parameter.GetDouble();
			break;
		}
		case ReflectionTypeData::Double2:
		case ReflectionTypeData::Double3:
		case ReflectionTypeData::Double4:
			break;
	}
}
