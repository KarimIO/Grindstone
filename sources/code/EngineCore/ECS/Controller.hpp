#pragma once

#include "Entity.hpp"
#include "Component.hpp"
#include "ComponentArray.hpp"
#include "System.hpp"

#include <unordered_map>
#include <iostream>

namespace Grindstone {
	namespace ECS {
		class Core;
		class Controller {
		public:
			Controller(Scene*);
			Entity createEntity();
			void removeEntity(Entity entity);

			void registerSystem(const char* type);
			void registerComponentType(const char *type);

			void* createComponent(Entity entity, const char* component_name) {
				auto it = component_map_.find(component_name);
				if (it != component_map_.end()) {
					return it->second->createGeneric(entity);
				}

				return nullptr;
			}

			template<typename Type>
			ComponentArray<Type>& registerComponentType() {
				using TypeArray = ComponentArray<Type>;
				TypeArray* tarr = new TypeArray();
				component_map_.emplace(TypeArray::component_name_, tarr);

				return *(tarr);
			}

			template<typename SystemClass>
			SystemClass& registerSystem() {
				SystemClass* sys = new SystemClass();
				system_map_.emplace(typeid(SystemClass).name(), sys);

				return *(sys);
			}

			template<typename ComponentStruct>
			ComponentStruct& createComponent(Entity entity) {
				ComponentType type = ComponentArray<ComponentStruct>::static_component_type_;
				IComponentArray* arr = nullptr; //component_map_.at(type);
				return ((ComponentArray<ComponentStruct>*)arr)->create(entity);
			}

			template<typename ComponentStruct>
			ComponentStruct &createComponent(Entity entity, ComponentStruct&&component) {
				ComponentType type = ComponentArray<ComponentStruct>::static_component_type_;
				IComponentArray *arr = component_map_[typeid(ComponentStruct).name()];
				return ((ComponentArray<ComponentStruct> *)arr)->create(entity, std::forward<ComponentStruct>(component));
			}

			template<typename ComponentStruct>
			ComponentArray<ComponentStruct>& getComponentArray() {
				const char * name = ComponentArray<ComponentStruct>::getComponentName();
				
				auto it = component_map_.find(name);
				if (it != component_map_.end()) {
					IComponentArray* arr = it->second;
					return *((ComponentArray<ComponentStruct>*)arr);
				}
				else {
					std::cout << "No such component\r\n";
				}
			}

			void update();
		private:
			std::unordered_map<std::string , IComponentArray* > component_map_;
			std::unordered_map<std::string, ISystem*> system_map_;
			Scene* scene_;
			ECS::Core* core_;
		};
	}
}
