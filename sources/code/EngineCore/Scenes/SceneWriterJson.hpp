#pragma once

#include <string>
#include <map>
#include <entt/entt.hpp>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "../ECS/Entity.hpp"
#include "../Reflection/TypeDescriptorStruct.hpp"

namespace Grindstone {
	namespace SceneManagement {
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
			void CopyDataArrayFloat(float* srcArray, rapidjson::SizeType count);
			void CopyDataArrayDouble(double* srcArray, rapidjson::SizeType count);
			void CopyDataArrayInt(int* srcArray, rapidjson::SizeType count);
			void ProcessComponentParameter(
				ECS::Entity entity,
				void* componentPtr,
				Reflection::TypeDescriptor_Struct& reflectionData,
				const char* parameterKey,
				rapidjson::Value& parameter
			);
			Scene* scene;
			rapidjson::StringBuffer documentStringBuffer;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> documentWriter = rapidjson::PrettyWriter<rapidjson::StringBuffer>(documentStringBuffer);
			const char* path;
		};
	}
}
