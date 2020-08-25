#pragma once

#include <vector>
#include <unordered_map>

#include "../pch.hpp"
#include "Entity.hpp"
#include "Component.hpp"

namespace Grindstone {
	namespace ECS {
		class IComponentArray {
		public:
			ComponentType component_type_;
			virtual void* createGeneric(Entity entity) = 0;
			virtual size_t getCount() = 0;
			virtual void reserve(unsigned int n) = 0;
			virtual void remove(Entity entity) = 0;
		};

		template<typename T>
		class ComponentArray : public IComponentArray {
		public:
			static IComponentArray* createComponentArray() {
				return new ComponentArray<T>();
			}

			virtual void reserve(unsigned int n) override {
				components_.reserve(n);
				entity_references_.reserve(n);
				entity_table_.reserve(n);
			}

			virtual void* createGeneric(Entity entity) override {
				size_t i = components_.size();
				components_.emplace_back();
				entity_references_.emplace_back(entity);
				entity_table_[entity] = i;

				return &components_.back();
			}

			virtual size_t getCount() override {
				return components_.size();
			}
			
			T& create(Entity entity) {
				size_t i = components_.size();
				components_.emplace_back();
				entity_references_.emplace_back(entity);
				entity_table_[entity] = i;

				return components_.back();
			}
			T& create(Entity entity, T&& t) {
				size_t i = components_.size();
				components_.emplace_back(std::forward<T>(t));
				entity_references_.emplace_back(entity);
				entity_table_[entity] = i;

				return components_.back();
			}
			bool contains(Entity entity) const {
				return entity_table_.find(entity) != entity_table_.end();
			}
			T& getComponent(Entity entity) {
				auto comp = entity_table_.find(entity);
				if (comp != entity_table_.end()) {
					return components_[comp->second];
				}
				
				throw std::runtime_error("Can't get component.");
			}
			virtual void remove(Entity entity) override {
				auto comp = entity_table_.find(entity);
				if (comp != entity_table_.end()) {
					const size_t index = comp->second;

					if (index < components_.size() - 1) {
						components_[index] = std::move(components_.back());
						entity_references_[index] = entity_references_.back();

						entity_table_[entity_references_[index]] = index;
					}

					components_.pop_back();
					entity_references_.pop_back();
					entity_table_.erase(entity);
				}
			}
			size_t size() const { return components_.size(); }
			T& operator[](size_t index) { return components_[index]; }
			Entity getEntity(size_t index) const { return entity_references_[index]; }
		public:
			static ComponentType type_id_;
			/*iterator                                       begin() { return iterator(&m_data[0]); }
			iterator                                       end() { return iterator(&m_data[m_size]); }

			const_iterator                                 cbegin() { return const_iterator(&m_data[0]); }
			const_iterator                                 cend() { return const_iterator(&m_data[m_size]); }

			reverse_iterator                               rbegin() { return reverse_iterator(&m_data[m_size - 1]); }
			reverse_iterator                               rend() { return reverse_iterator(&m_data[-1]); }

			const_reverse_iterator                         crbegin() { return const_reverse_iterator(&m_data[m_size - 1]); }
			const_reverse_iterator*/
		private:
			std::vector<T> components_;
			std::vector<Entity> entity_references_;
			std::unordered_map<Entity, size_t> entity_table_;
		};
	}
}