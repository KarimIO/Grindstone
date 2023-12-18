#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "SceneWriterJson.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

SceneWriterJson::SceneWriterJson(Scene* scene, const char* path) : scene(scene), path(path) {
	Save(path);
}

void SceneWriterJson::Save(const char* path) {
	documentWriter.StartObject();
	ProcessMeta();
	ProcessEntities();
	documentWriter.EndObject();

	std::filesystem::path dstPath = path;
	std::filesystem::path parentPath = dstPath.parent_path();
	if (parentPath != "") {
		if (std::filesystem::create_directories(parentPath)) {
			throw std::runtime_error("Failed to create directories to scene output path");
		}
	}

	const char* content = documentStringBuffer.GetString();
	std::ofstream file(path);
	file.write((const char*)content, strlen(content));
	file.flush();
	file.close();
}

void SceneWriterJson::ProcessMeta() {
	const char* name = scene->GetName();
	documentWriter.Key("name");
	documentWriter.String(name);
}

void SceneWriterJson::ProcessEntities() {
	documentWriter.Key("entities");
	documentWriter.StartArray();

	entt::registry& registry = scene->GetEntityRegistry();
	for (auto entityId : registry.storage<entt::entity>()) {
		ProcessEntity(registry, ECS::Entity(entityId, scene));
	}

	documentWriter.EndArray();
}

void SceneWriterJson::ProcessEntity(entt::registry& registry, ECS::Entity entity) {
	documentWriter.StartObject();

	documentWriter.Key("entityId");
	std::uint32_t entityUint = (std::uint32_t)entity.GetHandle();
	documentWriter.Uint(entityUint);

	documentWriter.Key("components");
	documentWriter.StartArray();

	auto& engineCore = EngineCore::GetInstance();
	ECS::ComponentRegistrar componentRegistrar = *engineCore.GetComponentRegistrar();
	for (auto& componentEntry : componentRegistrar) {
		const char* componentTypeName = componentEntry.first.c_str();
		auto componentReflectionData = componentEntry.second.GetComponentReflectionDataFn();
		auto tryGetComponentFn = componentEntry.second.TryGetComponentFn;

		void* outComponent = nullptr;
		if (entity.TryGetComponent(componentTypeName, outComponent)) {
			ProcessComponent(entity, componentTypeName, componentReflectionData, outComponent);
		}
	}

	documentWriter.EndArray();
	documentWriter.EndObject();
}

void SceneWriterJson::ProcessComponent(
	ECS::Entity entity,
	const char* componentTypeName,
	Grindstone::Reflection::TypeDescriptor_Struct& componentReflectionData,
	void* componentPtr
) {
	documentWriter.StartObject();

	documentWriter.Key("component");
	documentWriter.String(componentTypeName);

	documentWriter.Key("params");
	documentWriter.StartObject();

	auto& category = componentReflectionData.category;

	for (auto& param : category.members) {
		documentWriter.Key(param.storedName.c_str());
		SetParameter(param, componentPtr);
	}

	documentWriter.EndObject();
	documentWriter.EndObject();
}

void SceneWriterJson::SetParameter(Grindstone::Reflection::TypeDescriptor_Struct::Member param, void* componentPtr) {
	char* offset = ((char*)componentPtr + param.offset);

	switch (param.type->type) {
		default:
			documentWriter.Null();
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Quaternion:
			CopyDataArrayFloat((float*)offset, 4);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Bool:
			documentWriter.Bool(*(bool*)offset);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::String: {
			std::string& str = *(std::string*)offset;
			documentWriter.String(str.c_str());
			break;
		}
		case Reflection::TypeDescriptor::ReflectionTypeData::Int:
			documentWriter.Int(*(int*)offset);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Int2:
			CopyDataArrayInt((int*)offset, 2);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Int3:
			CopyDataArrayInt((int*)offset, 3);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Int4:
			CopyDataArrayInt((int*)offset, 4);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Float:
			documentWriter.Double(*(float*)offset);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Float2:
			CopyDataArrayFloat((float*)offset, 2);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Float3:
			CopyDataArrayFloat((float*)offset, 3);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Float4:
			CopyDataArrayFloat((float*)offset, 4);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Double:
			documentWriter.Double(*(double*)offset);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Double2:
			CopyDataArrayDouble((double*)offset, 2);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Double3:
			CopyDataArrayDouble((double*)offset, 3);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Double4:
			CopyDataArrayDouble((double*)offset, 4);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Vector: {
			std::vector<std::string>& vector = *(std::vector<std::string>*)offset;

			documentWriter.StartArray();
			for (auto& element : vector) {
				documentWriter.String(element.c_str());
			}
			documentWriter.EndArray();

			break;
		}
	}
}

void SceneWriterJson::CopyDataArrayFloat(float* srcArray, rapidjson::SizeType count) {
	documentWriter.StartArray();
	for (rapidjson::SizeType i = 0; i < count; ++i) {
		documentWriter.Double(srcArray[i]);
	}
	documentWriter.EndArray();
}

void SceneWriterJson::CopyDataArrayDouble(double* srcArray, rapidjson::SizeType count) {
	documentWriter.StartArray();
	for (rapidjson::SizeType i = 0; i < count; ++i) {
		documentWriter.Double(srcArray[i]);
	}
	documentWriter.EndArray();
}

void SceneWriterJson::CopyDataArrayInt(int* srcArray, rapidjson::SizeType count) {
	documentWriter.StartArray();
	for (rapidjson::SizeType i = 0; i < count; ++i) {
		documentWriter.Int(srcArray[i]);
	}
	documentWriter.EndArray();
}
