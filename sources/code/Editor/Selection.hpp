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
			void RemoveEntity(ECS::Entity);
			size_t GetSelectedEntityCount();
			ECS::Entity GetSingleSelectedEntity();

			void ClearFiles();
			void SetSelectedFile(std::filesystem::path path);
			void AddFile(std::filesystem::path path);
			void RemoveFile(std::filesystem::path path);
			size_t GetSelectedFileCount();
			std::filesystem::path GetSingleSelectedFile();
		public:
			std::set<ECS::Entity> selectedEntities;
			std::set<std::filesystem::path> selectedFiles;
		};
	}
}