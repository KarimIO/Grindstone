#pragma once

#include <string>
#include <map>
#include <entt/entt.hpp>
#include "rapidjson/document.h"

#include "../ECS/Entity.hpp"
#include "../Reflection/TypeDescriptorStruct.hpp"

namespace Grindstone {
	namespace SceneManagement {
		class Scene;

		class SceneLoaderJson {
		public:
			SceneLoaderJson(Scene*, const char* path);
		private:
			ECS::Entity createEntity();
			void* attachComponent(ECS::Entity entity, const char* componentName);
			bool load(const char* path);
		private:
			void ProcessMeta();
			void ProcessEntities();
			void ProcessEntity(rapidjson::GenericObject<false, rapidjson::Value>& entity);
			void ProcessComponent(ECS::Entity entity, rapidjson::GenericObject<false, rapidjson::Value>& component);
			void ProcessComponentParameter(
				ECS::Entity entity,
				void* componentPtr,
				Reflection::TypeDescriptor_Struct& reflectionData,
				const char* parameterKey,
				rapidjson::Value& parameter
			);
			Scene* scene;
			rapidjson::Document document;
			const char* path;
		};
	}
}
