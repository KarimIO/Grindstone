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
			bool IsEntitySelected(ECS::Entity);
			void RemoveEntity(ECS::Entity);
			size_t GetSelectedEntityCount();
			ECS::Entity GetSingleSelectedEntity();

			void ClearFiles();
			void SetSelectedFile(const std::filesystem::directory_entry& path);
			void AddFile(const std::filesystem::directory_entry& path);
			bool IsFileSelected(const std::filesystem::directory_entry& path);
			void RemoveFile(const std::filesystem::directory_entry& path);
			size_t GetSelectedFileCount();
			std::filesystem::directory_entry GetSingleSelectedFile();
		public:
			std::set<ECS::Entity> selectedEntities;
			std::set<std::filesystem::directory_entry> selectedFiles;
		};
	}
}