#pragma once

#include <vector>
#include <filesystem>

#include "CSharpProjectMetaData.hpp"

namespace Grindstone::Editor::ScriptBuilder {
	class CSharpBuildManager {
	public:
		void FinishInitialFileProcessing() const;
		void AddFileInitial(const std::filesystem::path& path);
		void OnFileAdded(const std::filesystem::path& path);
		void OnFileMoved(
			const std::filesystem::path& updatedPath,
			const std::filesystem::path& originalPath
		);
		void OnFileDeleted(const std::filesystem::path& path);
		static void OnFileModified(const std::filesystem::path& path);
	private:
		static void BuildProject();
		void CreateProjectsAndSolution() const;
		void CreateProject(const CSharpProjectMetaData& metaData) const;
		static void CreateSolution(const CSharpProjectMetaData& metaData);

		std::vector<std::filesystem::path> files;
	};
}
