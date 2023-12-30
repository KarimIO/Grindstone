#include <stdexcept>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "EngineCore/EngineCore.hpp"
#include "EngineCore/ECS/ComponentRegistrar.hpp"
#include "EngineCore/CoreComponents/Tag/TagComponent.hpp"
#include "SceneWriterJson.hpp"
#include "EngineCore/Utils/Utilities.hpp"
#include "Common/Math.hpp"
#include "Scene.hpp"

using namespace Grindstone;
using namespace Grindstone::SceneManagement;

void WriteParameter(SceneRapidjsonWriter& documentWriter, Reflection::TypeDescriptor* param, void* componentPtr);
void WriteArray(SceneRapidjsonWriter& documentWriter, void* memberPtr, Reflection::TypeDescriptor* member);
void WriteArrayFloat(SceneRapidjsonWriter& documentWriter, float* srcArray, rapidjson::SizeType count);
void WriteArrayDouble(SceneRapidjsonWriter& documentWriter, double* srcArray, rapidjson::SizeType count);
void WriteArrayInt(SceneRapidjsonWriter& documentWriter, int* srcArray, rapidjson::SizeType count);

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
	file.write(content, strlen(content));
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
	for (entt::entity entityId : registry.storage<entt::entity>()) {
		if (registry.valid(entityId)) {
			ProcessEntity(registry, ECS::Entity(entityId, scene));
		}
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
		char* dataPtr = static_cast<char*>(componentPtr) + param.offset;
		WriteParameter(documentWriter, param.type, dataPtr);
	}

	documentWriter.EndObject();
	documentWriter.EndObject();
}

void WriteParameter(SceneRapidjsonWriter& documentWriter, Reflection::TypeDescriptor* member, void* dataPtr) {
	switch (member->type) {
		default:
			documentWriter.Null();
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::AssetReference: {
			GenericAssetReference& assetRefPtr = *static_cast<GenericAssetReference*>(dataPtr);
			std::string uuidAsString = assetRefPtr.uuid.ToString();
			documentWriter.String(uuidAsString.c_str());
			break;
		}
		case Reflection::TypeDescriptor::ReflectionTypeData::Entity: {
			entt::entity entity = *static_cast<entt::entity*>(dataPtr);
			documentWriter.Uint(static_cast<unsigned int>(entity));
			break;
		}
		case Reflection::TypeDescriptor::ReflectionTypeData::Quaternion:
			WriteArrayFloat(documentWriter, static_cast<float*>(dataPtr), 4);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Bool:
			documentWriter.Bool(*static_cast<bool*>(dataPtr));
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::String: {
			std::string& str = *(std::string*)dataPtr;
			documentWriter.String(str.c_str());
			break;
		}
		case Reflection::TypeDescriptor::ReflectionTypeData::Int:
			documentWriter.Int(*static_cast<int*>(dataPtr));
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Int2:
			WriteArrayInt(documentWriter, static_cast<int*>(dataPtr), 2);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Int3:
			WriteArrayInt(documentWriter, static_cast<int*>(dataPtr), 3);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Int4:
			WriteArrayInt(documentWriter, static_cast<int*>(dataPtr), 4);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Float:
			documentWriter.Double(*static_cast<float*>(dataPtr));
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Float2:
			WriteArrayFloat(documentWriter, static_cast<float*>(dataPtr), 2);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Float3:
			WriteArrayFloat(documentWriter, static_cast<float*>(dataPtr), 3);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Float4:
			WriteArrayFloat(documentWriter, static_cast<float*>(dataPtr), 4);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Double:
			documentWriter.Double(*static_cast<double*>(dataPtr));
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Double2:
			WriteArrayDouble(documentWriter, static_cast<double*>(dataPtr), 2);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Double3:
			WriteArrayDouble(documentWriter, static_cast<double*>(dataPtr), 3);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Double4:
			WriteArrayDouble(documentWriter, static_cast<double*>(dataPtr), 4);
			break;
		case Reflection::TypeDescriptor::ReflectionTypeData::Vector:
			WriteArray(documentWriter, dataPtr, member);
			break;
	}
}

template<typename T>
inline void SetupArray(SceneRapidjsonWriter& documentWriter, void* memberPtr, Reflection::TypeDescriptor* member) {
	std::vector<T>& vector = *(std::vector<T>*)memberPtr;

	documentWriter.StartArray();
	for (size_t i = 0; i < vector.size(); ++i) {
		void* dataPtr = &vector[i];
		WriteParameter(documentWriter, member, dataPtr);
	}
	documentWriter.EndArray();
}

using ReflectionTypeData = Reflection::TypeDescriptor_Struct::ReflectionTypeData;
void WriteArray(SceneRapidjsonWriter& documentWriter, void* memberPtr, Reflection::TypeDescriptor* member) {
	Reflection::TypeDescriptor_StdVector* vectorTypeDescriptor = static_cast<Reflection::TypeDescriptor_StdVector*>(member);
	Reflection::TypeDescriptor* itemType = vectorTypeDescriptor->itemType;
	
	switch (itemType->type) {
	case ReflectionTypeData::Struct: break;
	case ReflectionTypeData::AssetReference:
		SetupArray<GenericAssetReference>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Bool:
		SetupArray<bool>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Quaternion:
		SetupArray<Math::Quaternion>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::String:
		SetupArray<std::string>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Vector:
		SetupArray<std::vector<char>>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Double:
		SetupArray<Math::Double>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Double2:
		SetupArray<Math::Double2>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Double3:
		SetupArray<Math::Double3>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Double4:
		SetupArray<Math::Double4>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Int:
		SetupArray<Math::Int>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Int2:
		SetupArray<Math::Int2>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Int3:
		SetupArray<Math::Int3>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Int4:
		SetupArray<Math::Int4>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Float:
		SetupArray<Math::Float>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Float2:
		SetupArray<Math::Float2>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Float3:
		SetupArray<Math::Float3>(documentWriter, memberPtr, itemType);
		break;
	case ReflectionTypeData::Float4:
		SetupArray<Math::Float4>(documentWriter, memberPtr, itemType);
		break;
	}
}

void WriteArrayFloat(SceneRapidjsonWriter& documentWriter, float* srcArray, rapidjson::SizeType count) {
	documentWriter.StartArray();
	for (rapidjson::SizeType i = 0; i < count; ++i) {
		documentWriter.Double(srcArray[i]);
	}
	documentWriter.EndArray();
}

void WriteArrayDouble(SceneRapidjsonWriter& documentWriter, double* srcArray, rapidjson::SizeType count) {
	documentWriter.StartArray();
	for (rapidjson::SizeType i = 0; i < count; ++i) {
		documentWriter.Double(srcArray[i]);
	}
	documentWriter.EndArray();
}

void WriteArrayInt(SceneRapidjsonWriter& documentWriter, int* srcArray, rapidjson::SizeType count) {
	documentWriter.StartArray();
	for (rapidjson::SizeType i = 0; i < count; ++i) {
		documentWriter.Int(srcArray[i]);
	}
	documentWriter.EndArray();
}
