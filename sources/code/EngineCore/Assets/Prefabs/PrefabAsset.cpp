#include <stdexcept>
#include <iostream>

#include <EngineCore/Profiling.hpp>
#include <EngineCore/Logger.hpp>
#include <EngineCore/EngineCore.hpp>
#include <EngineCore/ECS/SystemRegistrar.hpp>
#include <EngineCore/ECS/ComponentRegistrar.hpp>
#include <EngineCore/CoreComponents/Tag/TagComponent.hpp>
#include <EngineCore/CoreComponents/Parent/ParentComponent.hpp>
#include <EngineCore/CoreComponents/Transform/TransformComponent.hpp>
#include <EngineCore/Reflection/ComponentReflection.hpp>
#include <Common/PhysicsLayer.hpp>

#include "PrefabAsset.hpp"

using namespace Grindstone;

static void ParseMember(
	void* memberPtr,
	Reflection::TypeDescriptor* member,
	const PrefabAsset::ComponentDataDescriptor& parameter
) {
	using ReflectionTypeData = Grindstone::Reflection::TypeDescriptor::ReflectionTypeData;

	switch (member->type) {
	default:
		GPRINT_ERROR_V(LogSource::EngineCore, "Unhandled type '{}' in PrefabAsset!", member->GetFullName());
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
		bool& ptr = *static_cast<bool*>(memberPtr);
		ptr = parameter.GetBool();
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
		float& ptr = *static_cast<float*>(memberPtr);
		ptr = parameter.GetFloat();
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
		double& ptr = *static_cast<double*>(memberPtr);
		ptr = parameter.GetDouble();
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

static void ParseArray(void* memberPtr, Reflection::TypeDescriptor* itemType, const rapidjson::Value& parameter) {
	using ReflectionTypeData = Grindstone::Reflection::TypeDescriptor::ReflectionTypeData;

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
			const rapidjson::Value* elementIterator = srcArray.Begin();
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
		const rapidjson::Value* elementIterator = srcArray.Begin();
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

static void ParseFixedArray(void* memberPtr, Reflection::TypeDescriptor* member, const rapidjson::Value& parameter) {
	Reflection::TypeDescriptor_FixedArray* vectorTypeDescriptor = static_cast<Reflection::TypeDescriptor_FixedArray*>(member);
	ParseArray(memberPtr, vectorTypeDescriptor->itemType, parameter);
}

static void ParseVectorArray(void* memberPtr, Reflection::TypeDescriptor* member, const rapidjson::Value& parameter) {
	Reflection::TypeDescriptor_StdVector* vectorTypeDescriptor = static_cast<Reflection::TypeDescriptor_StdVector*>(member);
	ParseArray(memberPtr, vectorTypeDescriptor->itemType, parameter);
}

void Grindstone::PrefabAsset::InstantiateInWorldContext(Grindstone::WorldContextSet& worldContextSet, Grindstone::Uuid parentEntity) {
	Grindstone::EngineCore& engineCore = Grindstone::EngineCore::GetInstance();
	Grindstone::ECS::ComponentRegistrar* componentRegistry = engineCore.GetComponentRegistrar();

	entt::registry& registry = worldContextSet.GetEntityRegistry();
	for (PrefabAsset::EntityDescription& entityDesc : entitiesDescriptions) {
		entt::entity entityHandle = registry.create();

		ECS::Entity entity(entityHandle, &worldContextSet);

		for (PrefabAsset::ComponentDescriptor& componentDesc : entityDesc.components) {
			void* component = componentRegistry->CreateComponent(
				worldContextSet,
				componentDesc.componentType,
				entity
			);

		}
	}
}
