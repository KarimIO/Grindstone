#pragma once

#include <string>
#include <map>
#include <entt/entt.hpp>
#include <rapidjson/document.h>

#include <Common/ResourcePipeline/Uuid.hpp>

#include "../ECS/Entity.hpp"
#include "../Reflection/TypeDescriptorStruct.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;

		class SceneLoaderJson {
		public:
			SceneLoaderJson(Scene*, Grindstone::Uuid uuid);
		private:
			bool Load(Grindstone::Uuid uuid);
			void ProcessMeta();
			void ProcessEntities();

			void ProcessEntity(rapidjson::Value& entity);
			void ProcessComponent(ECS::Entity entity, rapidjson::Value& component);
			void ProcessComponentParameter(
				ECS::Entity entity,
				void* componentPtr,
				Reflection::TypeDescriptor_Struct& reflectionData,
				const char* parameterKey,
				rapidjson::Value& parameter
			);
			void ParseMember(void* memberPtr, Reflection::TypeDescriptor* member, rapidjson::Value& parameter);
			void ParseArray(void* memberPtr, Reflection::TypeDescriptor* member, rapidjson::Value& parameter);
		private:
			Scene* scene;
			rapidjson::Document document;
			Grindstone::Uuid uuid;
		};
	}
}
