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
			bool Load(const char* path);
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
			Scene* scene;
			rapidjson::Document document;
			const char* path;
		};
	}
}
