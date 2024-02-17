#pragma once

#include <vector>
#include <string>

#include "CSharpProjectMetaData.hpp"

namespace Grindstone::Editor::ScriptBuilder {
	class SolutionBuilder {
	public:
		void AddProject(const CSharpProjectMetaData& projectMetaData);
		void CreateSolution() const;
	private:
		static void OutputFile(const std::string& output);
		void WriteMainProjectSection(std::string& output) const;
		static void WriteSolutionConfigs(std::string& output);
		void WriteProjectConfigs(std::string& output) const;
		static void WriteSolutionProperties(std::string& output);

		std::vector<CSharpProjectMetaData> projects;
	};
}
