#pragma once

#include <string>
#include <map>
#include <entt/entt.hpp>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "../ECS/Entity.hpp"
#include "../Reflection/TypeDescriptorStruct.hpp"

namespace Grindstone::SceneManagement {
	using SceneRapidjsonWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>;
	class Scene;

	class SceneWriterJson {
	public:
		SceneWriterJson(Scene*, const char* path);
	private:
		void Save(const char* path);
		void ProcessMeta();
		void ProcessEntities();
		void ProcessEntity(entt::registry& registry, ECS::Entity entity);
		void ProcessComponent(
			ECS::Entity entity,
			const char* componentTypeName,
			Grindstone::Reflection::TypeDescriptor_Struct& componentReflectionData,
			void* outComponent
		);
		void SetParameter(Reflection::TypeDescriptor_Struct::Member param, void* componentPtr);
		void ProcessComponentParameter(
			ECS::Entity entity,
			void* componentPtr,
			Reflection::TypeDescriptor_Struct& reflectionData,
			const char* parameterKey,
			rapidjson::Value& parameter
		);
		void ParseArray(void* memberPtr, Reflection::TypeDescriptor* member);
	private:
		Scene* scene;
		rapidjson::StringBuffer documentStringBuffer;
		SceneRapidjsonWriter documentWriter = SceneRapidjsonWriter(documentStringBuffer);
		const char* path;
	};
}
