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
#include "EngineCore/CoreComponents/Mesh/MeshRendererComponent.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dManager.hpp"
#include "EngineCore/Assets/Mesh3d/Mesh3dRenderer.hpp"
#include "EngineCore/Assets/Materials/MaterialManager.hpp"

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

	auto& engineCore = EngineCore::GetInstance();
	auto materialManager = engineCore.materialManager;
	auto mesh3dManager = engineCore.mesh3dManager;
	auto mesh3dRenderer = engineCore.mesh3dRenderer;
	auto& registry = scene->GetEntityRegistry();

	auto& meshView = registry.view<MeshComponent>();
	meshView.each([&](MeshComponent& meshComponent) {
		meshComponent.mesh = &mesh3dManager->LoadMesh3d(meshComponent.meshPath.c_str());
	});

	auto& meshAndMeshRendererView = registry.view<MeshComponent, MeshRendererComponent>();
	meshAndMeshRendererView.each([&](
		MeshComponent& meshComponent,
		MeshRendererComponent& meshRendererComponent
	) {
		std::vector<Material*> materials;
		materials.resize(meshRendererComponent.materialPaths.size());
		for (size_t i = 0; i < meshRendererComponent.materialPaths.size(); ++i) {
			auto& materialPath = meshRendererComponent.materialPaths[i];
			materials[i] = &materialManager->LoadMaterial(
				mesh3dRenderer,
				materialPath.c_str()
			);
		}

		for (auto& submesh : meshComponent.mesh->submeshes) {
			if (submesh.materialIndex >= materials.size()) {
				continue;
			}

			Material* material = materials[submesh.materialIndex];
			material->renderables.push_back(&submesh);
		}
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
