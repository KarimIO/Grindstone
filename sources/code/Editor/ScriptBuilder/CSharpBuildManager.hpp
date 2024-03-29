#pragma once

#include <vector>
#include <filesystem>

#include "CSharpProjectMetaData.hpp"

namespace Grindstone {
	namespace Editor {
		namespace ScriptBuilder {
			class CSharpBuildManager {
			public:
				void FinishInitialFileProcessing();
				void AddFileInitial(const std::filesystem::path& path);
				void OnFileAdded(const std::filesystem::path& path);
				void OnFileMoved(
					const std::filesystem::path& originalPath,
					const std::filesystem::path& updatedPath
				);
				void OnFileDeleted(const std::filesystem::path& path);
				void OnFileModified(const std::filesystem::path& path);
			private:
				void BuildProject();
				void CreateProjectsAndSolution();
				void CreateProject(CSharpProjectMetaData metaData);
				void CreateSolution(CSharpProjectMetaData metaData);

				std::vector<std::filesystem::path> files;
			};
		}
	}
}
