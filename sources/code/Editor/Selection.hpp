#pragma once

#include <vector>
#include <set>
#include <filesystem>
#include "EngineCore/ECS/Entity.hpp"

namespace Grindstone {
	namespace Editor {
		class Selection {
		public:
			void Clear();
			void ClearEntities();
			void SetSelectedEntity(ECS::Entity);
			void AddEntity(ECS::Entity);
			bool IsEntitySelected(ECS::Entity) const;
			void RemoveEntity(ECS::Entity);
			size_t GetSelectedEntityCount() const;
			ECS::Entity GetSingleSelectedEntity() const;

			void ClearFiles();
			void SetSelectedFile(const std::filesystem::path& path);
			void AddFile(const std::filesystem::path& path);
			bool IsFileSelected(const std::filesystem::path& path) const;
			void RemoveFile(const std::filesystem::path& path);
			size_t GetSelectedFileCount() const;
			const std::filesystem::path& GetSingleSelectedFile() const;
		public:
			std::set<ECS::Entity> selectedEntities;
			std::set<std::filesystem::path> selectedFiles;
		};
	}
}
